package de.motis_project.app.io;

import android.os.Handler;

import com.neovisionaries.ws.client.WebSocket;
import com.neovisionaries.ws.client.WebSocketAdapter;
import com.neovisionaries.ws.client.WebSocketException;
import com.neovisionaries.ws.client.WebSocketFactory;
import com.neovisionaries.ws.client.WebSocketFrame;
import com.neovisionaries.ws.client.WebSocketState;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import de.motis_project.app.io.error.DisconnectedException;
import motis.Message;
import motis.MotisError;
import motis.MsgContent;

public class Server extends WebSocketAdapter {
    public interface Listener {
        void onMessage(Message m);

        void onConnect();

        void onDisconnect();
    }

    private static final int RECONNECT_INTERVAL = 5000;

    private final String url;
    private final List<Listener> listeners = new ArrayList<Listener>();
    private final Handler handler;
    private WebSocket ws;
    private long lastReceiveTimestamp = System.currentTimeMillis();
    private List<byte[]> sendQueue = new ArrayList<>();

    public Server(String url, Handler handler) {
        this.url = url;
        this.handler = handler;
    }

    public void connect() throws IOException {
        WebSocketFactory factory = new WebSocketFactory();
        factory.setConnectionTimeout(15000);

        ws = factory.createSocket(url);
        ws.addListener(this);
        ws.connectAsynchronously();
    }

    public boolean isConnected() {
        if (ws == null || lastReceiveTimestamp < System.currentTimeMillis() - 30000) {
            System.out.println("NOT CONNECTED");
            return false;
        } else {
            System.out.println((ws.getState() == WebSocketState.OPEN) ? "CONNECTED" : "NOT CONNECTED");
            return ws.getState() == WebSocketState.OPEN;
        }
    }

    protected boolean send(byte[] msg) throws DisconnectedException {
        if (!isConnected()) {
            sendQueue.add(msg);
            scheduleConnect();
            return false;
        }
        ws.sendPing();
        ws.sendBinary(msg);
        return true;
    }

    public void addListener(Listener l) {
        listeners.add(l);
    }

    public void removeListener(Listener l) {
        listeners.remove(l);
    }

    private void scheduleConnect() {
        System.out.println("Server.scheduleConnect");
        try {
            handler.postDelayed(() -> {
                try {
                    System.out.println("CONNECTING...");
                    connect();
                } catch (IOException e) {
                    System.out.println("CONNECT ERROR: " + e.getMessage());
                    scheduleConnect();
                }
            }, RECONNECT_INTERVAL);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onPongFrame(WebSocket websocket, WebSocketFrame frame) throws Exception {
        System.out.println("Server.onPongFrame");
    }

    @Override
    public void onBinaryMessage(WebSocket ws, byte[] buf) throws Exception {
        System.out.println("Server.onBinaryMessage");
        lastReceiveTimestamp = System.currentTimeMillis();

        Message msg = MessageBuilder.decode(buf);

        if (msg.contentType() == MsgContent.MotisError) {
            MotisError err = new MotisError();
            err = (MotisError) msg.content(err);
            System.out.println(
                    "RECEIVED ERROR: " + err.category() + ": " + err.reason());
        }

        synchronized (listeners) {
            for (Listener l : listeners) {
                l.onMessage(msg);
            }
        }
    }

    @Override
    public void onConnected(WebSocket websocket,
                            Map<String, List<String>> headers)
            throws Exception {
        System.out.println("Server.onConnected");

        lastReceiveTimestamp = System.currentTimeMillis();

        synchronized (sendQueue) {
            for (byte[] b : sendQueue) {
                send(b);
            }
            sendQueue.clear();
        }

        synchronized (listeners) {
            for (Listener l : listeners) {
                l.onConnect();
            }
        }
    }

    @Override
    public void onDisconnected(WebSocket websocket,
                               WebSocketFrame serverCloseFrame,
                               WebSocketFrame clientCloseFrame,
                               boolean closedByServer) throws Exception {
        System.out.println("Server.onDisconnected");
        synchronized (listeners) {
            for (Listener l : listeners) {
                l.onDisconnect();
            }
        }
        scheduleConnect();
    }

    @Override
    public void onConnectError(WebSocket websocket,
                               WebSocketException exception) throws Exception {
        scheduleConnect();
    }
}