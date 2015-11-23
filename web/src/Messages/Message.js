export default class Message {
  constructor(contentType, content, timeout = 1000) {
    this.contentType = contentType;
    this.content = content;
    this.timeout = timeout;
  }
}
