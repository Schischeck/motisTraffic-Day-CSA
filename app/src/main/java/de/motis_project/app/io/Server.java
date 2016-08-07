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

import motis.Message;

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

    public Server(String url, Handler handler) {
        this.url = url;
        this.handler = handler;
    }

    public void connect() throws IOException {
        WebSocketFactory factory = new WebSocketFactory();
        factory.setConnectionTimeout(60000);

        ws = factory.createSocket(url);
        ws.addListener(this);
        ws.connectAsynchronously();
    }

    public boolean isConnected() {
        if (ws == null) {
            return false;
        } else {
            return ws.getState() == WebSocketState.OPEN;
        }
    }

    public void send(byte[] msg) {
        if (isConnected()) {
            ws.sendBinary(msg);
        } else {
            System.out.println("not connected, unable to send message");
        }
    }

    public void addListener(Listener l) {
        listeners.add(l);
    }

    public void removeListener(Listener l) {
        listeners.remove(l);
    }

    private void scheduleConnect() {
        try {
            handler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    try {
                        connect();
                    } catch (IOException e) {
                        scheduleConnect();
                    }
                }
            }, RECONNECT_INTERVAL);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onBinaryMessage(WebSocket ws, byte[] buf) throws Exception {
        Message msg = MessageBuilder.decode(buf);
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