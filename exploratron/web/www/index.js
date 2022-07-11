var output = null;
var next_output = '';
var keys = [];

function terminal_output(text) {
  next_output += text;
}

document.onkeydown = function(evt) {
  evt = evt || window.event;
  // console.log('Receive key:', evt);
  if (evt.ctrlKey) {
    return;
  }
  if (evt.key.length == 1) {
    keys.push(evt.key.charCodeAt(0));
  } else {
    keys.push(evt.keyCode);
  }
};

function clear_screen() {
  output.innerHTML = '';
  next_output = '';
}

function next_key() {
  if (keys.length == 0) {
    return -1;
  }
  let key = keys.shift();
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
  },
};
