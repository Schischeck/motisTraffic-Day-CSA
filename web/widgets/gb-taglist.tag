<gb-taglist>
  <div if={opts.label} class="label">
    {opts.label}
  </div>
  <a href="#" class="tag" each={tag in opts.activeTags}>
    <i class="icon">{tag.icon}</i>
    <i class="remove icon" onclick={removeTag}>&#xE5C9;</i>
  </a>
  <div if={availableTags.length !== 0} href="#" class="tag outline" onclick={show}>
    <i class="icon">&#xE145;</i>
    <div if={isVisible} class="paper add">
      <a each={tag in availableTags} onclick={addTag} href="#" class="tag">
        <i class="icon">{tag.icon}</i>
      </a>
    </div>
  </div>

  <style scoped type="text/precss">
  .tag {
    display: block;
    float: left;
    height: 32px;
    width: 32px;
    background-color: precss.LIGHTEN("PRIMARY_COLOR", 20);
    color: white;
    margin: 5px;
    text-align: center;
    line-height: 32px;
    font-size: 32px;
    border-radius: 3px;
    box-shadow: 0 2px 4px rgba(0,0,0,.16);
  }

  .tag, .tag > * {
    cursor: pointer;
  }

  .tag.outline {
    border: 2px solid precss.LIGHTEN("PRIMARY_COLOR", 20);
    background-color: white;
    color: precss.LIGHTEN("PRIMARY_COLOR", 20);
  }

  .tag > .remove {
    position: relative;
    z-index: 1;
    top: -50px;
    left: 15px;
    color: white;
    font-size: .6em;
    text-shadow: 0 2px 2px rgba(0,0,0,.56);
    border-radius: 999px;
  }

  .tag > .remove:hover {
    text-shadow: 0 2px 4px rgba(0,0,0,.56);
  }

  .tag > .remove:active {
    text-shadow: 0 1px 2px rgba(0,0,0,.56);
  }

  .add {
    width: 32px;
    padding: 4px;
  }
  </style>

  <script>
  this.mixin('dismiss');
  this.removeTag = e => safeCall(this.opts.onremoveTag, e.item.tag)
  this.addTag = e => {
    safeCall(this.opts.onaddTag, e.item.tag);
    setTimeout(() => {
      this.hide();
      this.update();
    }, 0);
  }
  this.on('update', () => {
    const tagMatches = tag => otherTag => tag.id === otherTag.id
    const isNotActive = tag => this.opts.activeTags.find(tagMatches(tag)) === undefined
    this.availableTags = this.opts.availableTags.filter(isNotActive)
  })
  </script>
</gb-taglist>