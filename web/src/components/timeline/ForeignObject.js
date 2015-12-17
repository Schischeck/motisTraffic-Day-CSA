import SVG from 'svg.js';

SVG.ForeignObject = function() {
  this.constructor.call(this, SVG.create('foreignObject'));

  /* store type */
  this.type = 'foreignObject';
};

SVG.ForeignObject.prototype = new SVG.Shape;

SVG.extend(SVG.ForeignObject, {
  appendChild: function(child, attrs) {
    const newChild = typeof(child) === 'string' ? document.createElement(child) : child;
    if (typeof(attrs) === 'object') {
      Object.keys(attrs).forEach(a => {
        newChild.setAttribute(a, attrs[a]);
      });
    }
    this.node.appendChild(newChild);
    return this;
  },
  getChild: function(index) {
    return this.node.childNodes[index];
  }
});

SVG.extend(SVG.Container, {
  foreignObject: function(width, height) {
    return this.put(new SVG.ForeignObject).size(width === null ? 100 : width, height === null ? 100 : height);
  }
});
