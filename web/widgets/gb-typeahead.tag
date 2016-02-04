<gb-typeahead>
  <gb-input value={getValue()}
            label={opts.label}
            icon={opts.icon}
            placeholder={opts.placeholder}
            input-keyup={onKeyUp}>
    <yield/>
  </gb-input>
  <div if={isVisible && proposals.length !== 0} class="paper" onclick={preventDismiss}>
    <ul class="proposals">
      <li each={p, i in proposals}
          class={selected: (i === selectedIndex)}
          onclick={selectProposal}
          onmouseover={setSelectedIndex}>
        {parent.opts.displayProperty ? p[parent.opts.displayProperty] : p}
      </li>
    </ul>
  </div>

  <style type="text/precss">
  .proposals {
    list-style: none;
    padding: 0 !important;
    margin: 0;
    white-space: nowrap;
  }

  .proposals li {
    font-weight: 500;
    margin: 10px;
    padding: 16px;
    border-radius: 2px;
    display: block;
    width: 300px;
    overflow: hidden;
    text-overflow: ellipsis;
    font-size: 110%;
    cursor: pointer;
  }

  .proposals li.selected {
    background-color: precss.LIGHTEN("PRIMARY_COLOR", 85);
  }
  </style>

  <script>
  this.mixin('dismiss');

  this.proposals = [];
  this.selectedIndex = 0;

  this.on('dismiss', () => { this.selectedIndex = 0; });

  this.onKeyUp = e => {
    const input = e.target.value;
    if (this.getValue() !== input) {
      this.opts.displayProperty
          ? safeCall(this.opts.inputOnchange, { [this.opts.displayProperty]: input })
          : safeCall(this.opts.inputOnchange, input)
    }

    switch (e.keyCode) {
      // UP
      case 38:
        if (this.proposals.length !== 0) {
          this.selectedIndex = modulo(this.selectedIndex - 1, this.proposals.length);
          this.update();
        }
        break;

      // DOWN
      case 40:
        if (this.proposals.length !== 0) {
          this.selectedIndex = modulo(this.selectedIndex + 1, this.proposals.length);
          this.update();
        }
        break;

      // RETURN
      case 13:
      // ENTER
      case 14:
        this.selectProposal(this.selectedIndex);
        this.hide();
        return;

      // ESCAPE
      case 27:
        this.hide();
        return;

      default:
        if (this.lastInput === input || input.length < 3) {
          return;
        } else {
          opts.dataSrc(input).then(proposals => {
            this.lastInput = input;
            this.proposals = proposals;
            this.show();
          });
        }
    }
  }

  this.setSelectedIndex = (ev) => {
    this.selectedIndex = ev.item.i;
    this.update();
  }

  this.selectProposal = (element) => {
    const i = typeof element === 'number' ? element : element.item.i;
    const selected = this.proposals[i];
    safeCall(this.opts.inputOnchange, selected);
    this.hide();
    this.update();
  }

  this.getValue = () => {
    if (this.opts.displayProperty) {
      return this.opts.value[this.opts.displayProperty];
    } else {
      return this.opts.value;
    }
  }
  </script>
</gb-typeahead>
