riot.mixin('dismiss', {
  init: function() {
    this.isVisible = false;
    this.lastHandledClickTime = new Date();

    this.preventDismiss = () => {
      this.lastHandledClickTime = new Date();
    }

    this.hide = () => {
      this.preventDismiss();
      this.isVisible = false;
      document.removeEventListener('click', this.handleClickOutside);
      this.trigger('dismiss');
    }

    this.show = () => {
      this.preventDismiss();
      this.isVisible = true;
      document.addEventListener('click', this.handleClickOutside);
    }

    this.showAndUpdate = () => {
      this.show();
      this.update();
    }

    this.handleClickOutside = (e) => {
      if (!this.isVisible) {
        return;
      }
      if (!this.lastHandledClickTime ||
          new Date().getTime() - this.lastHandledClickTime.getTime() > 500) {
        this.hide();
        this.update();
      }
    }
  },
});

