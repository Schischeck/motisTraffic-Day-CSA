riot.mixin('i18n', {
  init: function() {
    const translations = {
      de: {
        'Search': 'Suchen',
        'From': 'Start',
        'To': 'Ziel',
        'Date': 'Datum',
        'Time': 'Uhrzeit',
        'Departure': 'Abfahrt',
        'Arrival': 'Ankunft',
        'Transports at the Start': 'Verkehrsmittel am Start',
        'Transports at the Destination': 'Verkehrsmittel am Ziel',
        'Show results of the last search': 'Ergebnisse der letzten Suche anzeigen',
        'An error occured.': 'Es ist ein Fehler aufgetreten.',
        'No connections found.': 'Keine Verbindungen gefunden'
      },
      en: {
      }
    };
    const obs = riot.observable();
    this.on = obs.on;
    this.off = obs.off;
    this.trigger = obs.trigger;
    this.on('lang', (lang) => {
      lang = 'de'
      this.currentLocale = lang || navigator.language.substring(0, 2);
      const dict = translations[this.currentLocale] || translations.en;
      this.translate = libTranslate.getTranslationFunction(dict);
      this.trigger('update');
    });
    this.trigger('lang');
  },
});

