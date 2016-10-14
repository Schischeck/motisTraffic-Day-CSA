package de.motis_project.app.io;

import android.os.Handler;

import com.google.flatbuffers.Table;

import java.util.Date;

import de.motis_project.app.io.error.DisconnectedException;
import de.motis_project.app.io.error.MotisErrorException;
import de.motis_project.app.io.error.UnexpectedMessageTypeException;
import motis.Message;
import motis.MotisError;
import motis.MsgContent;
import motis.guesser.StationGuesserResponse;
import motis.lookup.LookupScheduleInfoResponse;
import motis.routing.RoutingResponse;
import rx.Observable;
import rx.Subscriber;

public class MotisServer extends Server {
    class ResponseListener<T extends Table> implements Server.Listener {
        final Subscriber<? super T> subscriber;
        final int responseId;
        final T response;
        final byte responseType;

        ResponseListener(
                final Subscriber<? super T> subscriber,
                byte responseType,
                T response,
                int responseId) {
            this.responseId = responseId;
            this.subscriber = subscriber;
            this.response = response;
            this.responseType = responseType;
            addListener(this);
        }

        @Override
        public void onMessage(Message m) {
            if (subscriber.isUnsubscribed()) {
                removeListener(this);
                return;
            }

            if (m.id() != responseId) {
                return;
            }

            if (m.contentType() == responseType) {
                m.content(response);
                subscriber.onNext(response);
            } else if (m.contentType() == MsgContent.MotisError) {
                MotisError err = new MotisError();
                m.content(err);
                subscriber.onError(
                        new MotisErrorException(err.category(), err.reason(),
                                                err.errorCode()));
            } else {
                subscriber.onError(new UnexpectedMessageTypeException());
            }

            subscriber.onCompleted();
            removeListener(this);
        }

        @Override
        public void onConnect() {
        }

        @Override
        public void onDisconnect() {
            subscriber.onError(new DisconnectedException());
            subscriber.onCompleted();
        }
    }

    class ResponseSubscription<T extends Table>
            implements Observable.OnSubscribe<T> {
        final byte[] request;
        final int responseId;
        final T response;
        final byte responseType;
        ResponseListener<T> listener;

        ResponseSubscription(byte[] request, byte responseType, T response,
                             int responseId) {
            this.request = request;
            this.responseId = responseId;
            this.response = response;
            this.responseType = responseType;
        }

        @Override
        public void call(final Subscriber<? super T> subscriber) {
            System.out.println("CALLED!!!");
            try {
                send(request);
                listener =
                        new ResponseListener<T>(subscriber, responseType,
                                                response,
                                                responseId);
            } catch (DisconnectedException e) {
                subscriber.onError(e);
            }
        }
    }

    private int nextMsgId = 0;

    public MotisServer(String url, Handler handler) {
        super(url, handler);
    }

    public Observable<StationGuesserResponse> guess(String input) {
        final int id = ++nextMsgId;
        return Observable.create(new ResponseSubscription<>(
                MessageBuilder.guess(id, input),
                MsgContent.StationGuesserResponse,
                new StationGuesserResponse(),
                id));
    }

    public Observable<RoutingResponse> route(
            String fromId, String toId,
            boolean isArrival,
            Date intervalBegin, Date intervalEnd,
            int min_connection_count) {
        final int id = ++nextMsgId;
        return Observable.create(new ResponseSubscription<>(
                MessageBuilder.route(id, fromId, toId, isArrival,
                                     intervalBegin, intervalEnd,
                                     min_connection_count),
                MsgContent.RoutingResponse,
                new RoutingResponse(),
                id));
    }

    public Observable<LookupScheduleInfoResponse> scheduleInfo() {
        final int id = ++nextMsgId;
        return Observable.create(new ResponseSubscription<>(
                MessageBuilder.scheduleInfo(id),
                MsgContent.LookupScheduleInfoResponse,
                new LookupScheduleInfoResponse(), id));
    }
}
