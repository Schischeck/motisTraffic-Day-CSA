<i18n>
  <yield />

  <script>
    this.mixin('i18n');
    this.on('update', () => {
      this.root.innerHTML = this.translate(this.root.innerHTML);
    });
  </script>
</i18n>
