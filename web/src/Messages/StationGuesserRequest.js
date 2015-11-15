import Message from './Message';

export default class StationGuesserRequest extends Message {
  constructor(input, guessCount) {
    super('StationGuesserRequest', {
      'input': input,
      'guess_count': guessCount
    });
  }
}
