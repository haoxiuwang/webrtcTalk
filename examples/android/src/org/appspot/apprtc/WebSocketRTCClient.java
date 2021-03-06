/*
 * libjingle
 * Copyright 2014 Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.appspot.apprtc;

import org.appspot.apprtc.RoomParametersFetcher.RoomParametersFetcherEvents;
import org.appspot.apprtc.WebSocketChannelClient.WebSocketChannelEvents;
import org.appspot.apprtc.WebSocketChannelClient.WebSocketConnectionState;
import org.appspot.apprtc.util.AsyncHttpURLConnection;
import org.appspot.apprtc.util.AsyncHttpURLConnection.AsyncHttpEvents;
import org.appspot.apprtc.util.LooperExecutor;

import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;
import org.webrtc.IceCandidate;
import org.webrtc.SessionDescription;

/**
 * Negotiates signaling for chatting with apprtc.appspot.com "rooms".
 * Uses the client<->server specifics of the apprtc AppEngine webapp.
 *
 * <p>To use: create an instance of this object (registering a message handler) and
 * call connectToRoom().  Once room connection is established
 * onConnectedToRoom() callback with room parameters is invoked.
 * Messages to other party (with local Ice candidates and answer SDP) can
 * be sent after WebSocket connection is established.
 */
public class WebSocketRTCClient implements AppRTCClient,
    WebSocketChannelEvents {
  private static final String TAG = "WSRTCClient";
  private static final String ROOM_JOIN = "join";
  private static final String ROOM_MESSAGE = "message";
  private static final String ROOM_LEAVE = "leave";

  private enum ConnectionState {
    NEW, CONNECTED, CLOSED, ERROR
  };
  private enum MessageType {
    MESSAGE, LEAVE
  };
  private final LooperExecutor executor;
  private boolean initiator;
  private SignalingEvents events;
  private WebSocketChannelClient wsClient;
  private ConnectionState roomState;
  private RoomConnectionParameters connectionParameters;
  private String messageUrl;
  private String leaveUrl;

  public WebSocketRTCClient(SignalingEvents events, LooperExecutor executor) {
    this.events = events;
    this.executor = executor;
    roomState = ConnectionState.NEW;
  }

  // --------------------------------------------------------------------
  // AppRTCClient interface implementation.
  // Asynchronously connect to an AppRTC room URL using supplied connection
  // parameters, retrieves room parameters and connect to WebSocket server.
  @Override
  public void connectToRoom(RoomConnectionParameters connectionParameters) {
    this.connectionParameters = connectionParameters;
    executor.requestStart();
    executor.execute(new Runnable() {
      @Override
      public void run() {
        connectToRoomInternal();
      }
    });
  }

  @Override
  public void disconnectFromRoom() {
    executor.execute(new Runnable() {
      @Override
      public void run() {
        disconnectFromRoomInternal();
      }
    });
    executor.requestStop();
  }

  // Connects to room - function runs on a local looper thread.
  private void connectToRoomInternal() {
    String connectionUrl = getConnectionUrl(connectionParameters);
    Log.d(TAG, "Connect to room: " + connectionUrl);
    roomState = ConnectionState.NEW;
    wsClient = new WebSocketChannelClient(executor, this);

    RoomParametersFetcherEvents callbacks = new RoomParametersFetcherEvents() {
      @Override
      public void onSignalingParametersReady(
          final SignalingParameters params) {
        WebSocketRTCClient.this.executor.execute(new Runnable() {
          @Override
          public void run() {
            WebSocketRTCClient.this.signalingParametersReady(params);
          }
        });
      }

      @Override
      public void onSignalingParametersError(String description) {
        WebSocketRTCClient.this.reportError(description);
      }
    };

    new RoomParametersFetcher(connectionParameters.loopback, connectionUrl,
        null, callbacks).makeRequest();
  }

  // Disconnect from room and send bye messages - runs on a local looper thread.
  private void disconnectFromRoomInternal() {
    Log.d(TAG, "Disconnect. Room state: " + roomState);
    if (roomState == ConnectionState.CONNECTED) {
      Log.d(TAG, "Closing room.");
      sendPostMessage(MessageType.LEAVE, leaveUrl, null);
    }
    roomState = ConnectionState.CLOSED;
    if (wsClient != null) {
      wsClient.disconnect(true);
    }
  }

  // Helper functions to get connection, post message and leave message URLs
  private String getConnectionUrl(
      RoomConnectionParameters connectionParameters) {
    return connectionParameters.roomUrl + "/" + ROOM_JOIN + "/"
        + connectionParameters.roomId;
  }

  private String getMessageUrl(RoomConnectionParameters connectionParameters,
      SignalingParameters signalingParameters) {
    return connectionParameters.roomUrl + "/" + ROOM_MESSAGE + "/"
      + connectionParameters.roomId + "/" + signalingParameters.clientId;
  }

  private String getLeaveUrl(RoomConnectionParameters connectionParameters,
      SignalingParameters signalingParameters) {
    return connectionParameters.roomUrl + "/" + ROOM_LEAVE + "/"
        + connectionParameters.roomId + "/" + signalingParameters.clientId;
  }

  // Callback issued when room parameters are extracted. Runs on local
  // looper thread.
  private void signalingParametersReady(
      final SignalingParameters signalingParameters) {
    Log.d(TAG, "Room connection completed.");
    if (connectionParameters.loopback
        && (!signalingParameters.initiator
            || signalingParameters.offerSdp != null)) {
      reportError("Loopback room is busy.");
      return;
    }
    if (!connectionParameters.loopback
        && !signalingParameters.initiator
        && signalingParameters.offerSdp == null) {
      Log.w(TAG, "No offer SDP in room response.");
    }
    initiator = signalingParameters.initiator;
    messageUrl = getMessageUrl(connectionParameters, signalingParameters);
    leaveUrl = getLeaveUrl(connectionParameters, signalingParameters);
    Log.d(TAG, "Message URL: " + messageUrl);
    Log.d(TAG, "Leave URL: " + leaveUrl);
    roomState = ConnectionState.CONNECTED;

    // Fire connection and signaling parameters events.
    events.onConnectedToRoom(signalingParameters);

    // Connect to WebSocket server.
    wsClient.connect(signalingParameters.wssUrl, signalingParameters.wssPostUrl,
        connectionParameters.roomId, signalingParameters.clientId);
  }

  // Send local offer SDP to the other participant.
  @Override
  public void sendOfferSdp(final SessionDescription sdp) {
    executor.execute(new Runnable() {
      @Override
      public void run() {
        if (roomState != ConnectionState.CONNECTED) {
          reportError("Sending offer SDP in non connected state.");
          return;
        }
        JSONObject json = new JSONObject();
        jsonPut(json, "sdp", sdp.description);
        jsonPut(json, "type", "offer");
        sendPostMessage(MessageType.MESSAGE, messageUrl, json.toString());
        if (connectionParameters.loopback) {
          // In loopback mode rename this offer to answer and route it back.
          SessionDescription sdpAnswer = new SessionDescription(
              SessionDescription.Type.fromCanonicalForm("answer"),
              sdp.description);
          events.onRemoteDescription(sdpAnswer);
        }
      }
    });
  }

  // Send local answer SDP to the other participant.
  @Override
  public void sendAnswerSdp(final SessionDescription sdp) {
    executor.execute(new Runnable() {
      @Override
      public void run() {
        if (connectionParameters.loopback) {
          Log.e(TAG, "Sending answer in loopback mode.");
          return;
        }
        JSONObject json = new JSONObject();
        jsonPut(json, "sdp", sdp.description);
        jsonPut(json, "type", "answer");
        wsClient.send(json.toString());
      }
    });
  }

  // Send Ice candidate to the other participant.
  @Override
  public void sendLocalIceCandidate(final IceCandidate candidate) {
    executor.execute(new Runnable() {
      @Override
      public void run() {
        JSONObject json = new JSONObject();
        jsonPut(json, "type", "candidate");
        jsonPut(json, "label", candidate.sdpMLineIndex);
        jsonPut(json, "id", candidate.sdpMid);
        jsonPut(json, "candidate", candidate.sdp);
        if (initiator) {
          // Call initiator sends ice candidates to GAE server.
          if (roomState != ConnectionState.CONNECTED) {
            reportError("Sending ICE candidate in non connected state.");
            return;
          }
          sendPostMessage(MessageType.MESSAGE, messageUrl, json.toString());
          if (connectionParameters.loopback) {
            events.onRemoteIceCandidate(candidate);
          }
        } else {
          // Call receiver sends ice candidates to websocket server.
          wsClient.send(json.toString());
        }
      }
    });
  }

  // --------------------------------------------------------------------
  // WebSocketChannelEvents interface implementation.
  // All events are called by WebSocketChannelClient on a local looper thread
  // (passed to WebSocket client constructor).
  @Override
  public void onWebSocketOpen() {
    Log.d(TAG, "Websocket connection completed. Registering...");
    wsClient.register();
  }

  @Override
  public void onWebSocketMessage(final String msg) {
    if (wsClient.getState() != WebSocketConnectionState.REGISTERED) {
      Log.e(TAG, "Got WebSocket message in non registered state.");
      return;
    }
    try {
      JSONObject json = new JSONObject(msg);
      String msgText = json.getString("msg");
      String errorText = json.optString("error");
      if (msgText.length() > 0) {
        json = new JSONObject(msgText);
        String type = json.optString("type");
        if (type.equals("candidate")) {
          IceCandidate candidate = new IceCandidate(
              json.getString("id"),
              json.getInt("label"),
              json.getString("candidate"));
          events.onRemoteIceCandidate(candidate);
        } else if (type.equals("answer")) {
          if (initiator) {
            SessionDescription sdp = new SessionDescription(
                SessionDescription.Type.fromCanonicalForm(type),
                json.getString("sdp"));
            events.onRemoteDescription(sdp);
          } else {
            reportError("Received answer for call initiator: " + msg);
          }
        } else if (type.equals("offer")) {
          if (!initiator) {
            SessionDescription sdp = new SessionDescription(
                SessionDescription.Type.fromCanonicalForm(type),
                json.getString("sdp"));
            events.onRemoteDescription(sdp);
          } else {
            reportError("Received offer for call receiver: " + msg);
          }
        } else if (type.equals("bye")) {
          events.onChannelClose();
        } else {
          reportError("Unexpected WebSocket message: " + msg);
        }
      } else {
        if (errorText != null && errorText.length() > 0) {
          reportError("WebSocket error message: " + errorText);
        } else {
          reportError("Unexpected WebSocket message: " + msg);
        }
      }
    } catch (JSONException e) {
      reportError("WebSocket message JSON parsing error: " + e.toString());
    }
  }

  @Override
  public void onWebSocketClose() {
    events.onChannelClose();
  }

  @Override
  public void onWebSocketError(String description) {
    reportError("WebSocket error: " + description);
  }

  // --------------------------------------------------------------------
  // Helper functions.
  private void reportError(final String errorMessage) {
    Log.e(TAG, errorMessage);
    executor.execute(new Runnable() {
      @Override
      public void run() {
        if (roomState != ConnectionState.ERROR) {
          roomState = ConnectionState.ERROR;
          events.onChannelError(errorMessage);
        }
      }
    });
  }

  // Put a |key|->|value| mapping in |json|.
  private static void jsonPut(JSONObject json, String key, Object value) {
    try {
      json.put(key, value);
    } catch (JSONException e) {
      throw new RuntimeException(e);
    }
  }

  // Send SDP or ICE candidate to a room server.
  private void sendPostMessage(
      final MessageType messageType, final String url, final String message) {
    String logInfo = url;
    if (message != null) {
      logInfo += ". Message: " + message;
    }
    Log.d(TAG, "C->GAE: " + logInfo);
    AsyncHttpURLConnection httpConnection = new AsyncHttpURLConnection(
      "POST", url, message, new AsyncHttpEvents() {
        @Override
        public void onHttpError(String errorMessage) {
          reportError("GAE POST error: " + errorMessage);
        }

        @Override
        public void onHttpComplete(String response) {
          if (messageType == MessageType.MESSAGE) {
            try {
              JSONObject roomJson = new JSONObject(response);
              String result = roomJson.getString("result");
              if (!result.equals("SUCCESS")) {
                reportError("GAE POST error: " + result);
              }
            } catch (JSONException e) {
              reportError("GAE POST JSON error: " + e.toString());
            }
          }
        }
      });
    httpConnection.send();
  }
}
