package de.motis_project.app;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SocketChannel;

import org.iq80.snappy.Snappy;

import com.google.flatbuffers.FlatBufferBuilder;

import motis.Message;

public class Server {
    private static final int SIZE_OF_SIZETYPE = 4;

    public Thread timeoutTimer;
    private final String host;
    private final int port;
    private SocketChannel socket = null;

    private static Server instance = null;

    public static synchronized Server instance() {
        if (instance == null) {
            instance = new Server("bellman.algo.informatik.tu-darmstadt.de", 7000);
        }
        return instance;
    }

    public synchronized Message request(FlatBufferBuilder b) throws Exception {
        return request(b, 30000);
    }

    public synchronized Message request(FlatBufferBuilder b, int timeoutInMilliseconds) throws Exception {
        startTimeout(timeoutInMilliseconds);
        try {
            System.out.println("isOpen: " + (socket != null && socket.isOpen()));
            if (socket == null || !socket.isConnected()) {
                connect();
            }
            writeRequest(b);
            return readReply();
        } catch (Exception e) {
            e.printStackTrace();
            if (socket != null) {
                socket.close();
                socket = null;
            }
            throw e;
        } finally {
            cancelTimeout();
        }
    }

    private Server(String host, int port) {
        this.host = host;
        this.port = port;
    }

    void disconnect() {
        try {
            socket.close();
        } catch (IOException e) {
        }
    }

    private void startTimeout(final int milliseconds) {
        timeoutTimer = new Thread(new Runnable() {
            public void run() {
                try {
                    Thread.sleep(milliseconds);
                    socket.close();
                } catch (InterruptedException e) {
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
        timeoutTimer.start();
    }

    private void nop() throws InterruptedException {
        Thread.sleep(5);
    }

    private void cancelTimeout() {
        if (timeoutTimer != null) {
            timeoutTimer.interrupt();
        }
    }

    private void connect() throws Exception {
        System.out.println("connecting");
        socket = SocketChannel.open();
        socket.connect(new InetSocketAddress(host, port));
        socket.configureBlocking(false);

        int nopCount = 0;
        while (!socket.finishConnect()) {
            nop();
            if (nopCount++ > 2000) {
                throw new IOException("connect timeout");
            }
        }
    }

    private void writeRequest(FlatBufferBuilder b) throws Exception {
        byte[] compressed = Snappy.compress(b.sizedByteArray());
        ByteBuffer msgBuf = ByteBuffer.wrap(compressed);
        ByteBuffer sizeBuf = ByteBuffer.allocate(SIZE_OF_SIZETYPE);
        sizeBuf.order(ByteOrder.BIG_ENDIAN);
        sizeBuf.putInt(compressed.length);
        sizeBuf.flip();

        int nopCount = 0;
        while (sizeBuf.hasRemaining()) {
            socket.write(sizeBuf);
            nop();
            if (nopCount++ >= 10) {
                throw new IOException("write timeout");
            }
        }
        while (msgBuf.hasRemaining()) {
            socket.write(msgBuf);
            nop();
        }

        System.out.println("write complete");
    }

    private Message readReply() throws Exception {
        ByteBuffer sizeBuf = ByteBuffer.allocate(SIZE_OF_SIZETYPE);

        int nopCount = 0;
        while (sizeBuf.hasRemaining()) {
            int ret = socket.read(sizeBuf);
            nop();
            if (nopCount++ >= 1000) {
                throw new IOException("read timeout");
            }
            if (ret == -1 && sizeBuf.hasRemaining()) {
                throw new IOException("socket closed");
            }
        }

        sizeBuf.flip();
        int msgSize = sizeBuf.getInt();

        ByteBuffer msgBuf = ByteBuffer.allocate(msgSize);
        while (msgBuf.hasRemaining()) {
            int ret = socket.read(msgBuf);
            nop();
            if (ret == -1 && sizeBuf.hasRemaining()) {
                throw new IOException("socket closed");
            }
        }
        msgBuf.flip();

        ByteBuffer uncompressed = ByteBuffer.wrap(Snappy.uncompress(msgBuf.array(), 0, msgSize));
        return Message.getRootAsMessage(uncompressed);
    }
}
