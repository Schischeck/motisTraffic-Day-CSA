import Message from './Message';

export default class StationGuesserRequest extends Message {
  constructor(input, guessCount = 7) {
    super('StationGuesserRequest', {
      'input': input,
      'guess_count': guessCount
    });
  }
}
