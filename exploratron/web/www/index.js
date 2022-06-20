// var term = null;
var output = null;
var next_output = '';
var keys = [];

function terminal_output(text) {
  // console.log('terminal_output:', text);
  //  term.write(text);
  next_output += text;
}

document.onkeydown = function(evt) {
  evt = evt || window.event;
  //console.log('Receive key:', evt);
  if (evt.key.length == 1) {
    keys.push(evt.key.charCodeAt(0));
  } else {
    keys.push(evt.keyCode);
  }
};

function clear_screen() {
  // console.log('clear_screen');
  output.innerHTML = '';
  next_output = '';
  // term.clear();
}

function next_key() {
  if (keys.length == 0) {
    return -1;
  }
  let key = keys.shift();
  //console.log('Send key', keys, '=', key);
  return key;
}

let color_mapping = {
  37: 'white',
  31: 'red',
  34: 'LightBlue',
  32: 'green',
  90: 'gray',
  35: 'violet',
  33: 'yellow',
};

function refresh_screen() {
  let html = '';
  let i = 0;
  const n = next_output.length;
  let color = -1;
  while (i < n) {
    if (next_output.charCodeAt(i) == 0x1B) {
      if (color != -1) {
        html += '</span>';
      }
      i++;
      color = next_output.charCodeAt(i);
      html += '<span style="color:' + color_mapping[color] + '">';
    } else {
      html += next_output.charAt(i);
    }
    i++;
  }
  if (color != -1) {
    html += '</span>';
  }
  output.innerHTML = html;
  next_output = '';
}

var Module = {
  print: terminal_output,
  printErr: terminal_output,
  preRun: [function() {}],
  onRuntimeInitialized: function() {
    output = document.getElementById('output');

    /*
    // setOption(key: 'fontSize' | 'letterSpacing' | 'lineHeight' |
    // 'tabStopWidth' | 'scrollback', value: number): void;
    //term.setOption('fontSize', 14);
    term = new Terminal({
      rows: 40,
      cols: 120,
      fontSize: 14,
      fontFamily: 'CascadiaMono',
      letterSpacing: -4,
      // fontFamily: 'Roboto Mono',
      //fontFamily: 'Ubuntu Mono, courier-new, courier, monospace',
      //enableBold: true,
      //letterSpacing: -0.7,
      //lineHeight:0.9,
      // convertEol: true,
    });

    // const fitAddon = new FitAddon.FitAddon();
    // term.loadAddon(fitAddon);
    // term.loadAddon(new B.XtermWebfont())
    term.open(document.getElementById('terminal'));
    // term.loadWebfontAndOpen(document.getElementById('terminal'))
    term.onKey(function(ev) {
      let key = ev.key;
      for (var i = 0; i < key.length; i++) {
        keys.push(key.charAt(i));
      }
    });
    term.focus();
    // fitAddon.fit();
    */
  },
};
