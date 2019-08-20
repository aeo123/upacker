package ltd.zlink.ae_tracer.msg;

public interface MsgCallback {
    void onMsgPrased(byte[] data, int len);

    void onMsgFailed();
}
