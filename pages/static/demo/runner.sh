# Run this script to create the demo.
#
# Tools:
#
# - https://github.com/PierreMarchand20/asciinema_automation/tree/main
#        python3 -m venv asciinema-automation
#        source asciinema-automation/bin/activate
#        pip install asciinema-automation
# - Asciinema
#        aptitude install pipx
#        pipx install asciinema
# - bat
#        aptitude install bat; ln -s /usr/bin/batcat ~/bin/bat
# - agg
#        curl -L -O https://github.com/asciinema/agg/releases/download/v1.5.0/agg-x86_64-unknown-linux-gnu
# - gifsicle
#        aptitude install gifsicle
# - Fira Mono
#        https://fonts.google.com/specimen/Fira+Mono
#        sudo mv FiraMono-* /usr/local/share/fonts/
#        sudo fc-cache

set -e

export PCRE2_VERSION=10.45
touch ./.hushlogin

if [ "$ISDARK" = 1 ]; then
  COLTAG="dark"
  export BAT_THEME=OneHalfDark
else
  COLTAG="light"
  export BAT_THEME=OneHalfLight
fi

rm -f *.cast
rm -rf ./pcre2

SHELL=/bin/bash HOME=`pwd` asciinema-automation --standard-deviation=0 -aa="--cols=80" demo.sh demo-$COLTAG.cast

head -n-2 demo-$COLTAG.cast > demo-$COLTAG.trimmed.cast
if ! tail -n1 demo-$COLTAG.trimmed.cast | grep '")"' > /dev/null; then
  head -n-3 demo-$COLTAG.cast > demo-$COLTAG.trimmed.cast
fi
if ! tail -n1 demo-$COLTAG.trimmed.cast | grep '")"' > /dev/null; then
  echo "Did not see ) on last line"
  exit 1
fi

# light theme - background: f6f8fa, font: 262626, blue: 006094 (sec), light blue: 0085cc (prim)
# dark theme - background: 151b23, font: f0f6fc, blue: 0075b4 (sec), light blue: 33b8ff (prim)

# Original GitHub themes:
#DARK_THEME="--theme 171b21,eceff4,0e1116,f97583,a2fca2,fabb72,7db4f9,c4a0f5,1f6feb,eceff4,6a737d,bf5a64,7abf7a,bf8f57,608bbf,997dbf,195cbf,b9bbbf"
#LGHT_THEME="--theme eceff4,171b21,0e1116,f97583,a2fca2,fabb72,7db4f9,c4a0f5,1f6feb,eceff4,6a737d,bf5a64,7abf7a,bf8f57,608bbf,997dbf,195cbf,b9bbbf"

# PCRE2 colors:
#                                                             blue          blue                                      lblue         lblue
DARK_THEME="--theme 151b23,f0f6fc,0e1116,f97583,a2fca2,fabb72,0075b4,c4a0f5,0075b4,f0f6fc,6a737d,bf5a64,7abf7a,bf8f57,33b8ff,997dbf,33b8ff,b9bbbf"
LGHT_THEME="--theme f6f8fa,262626,0e1116,f97583,a2fca2,fabb72,006094,c4a0f5,006094,f0f6fc,6a737d,bf5a64,7abf7a,bf8f57,0085cc,997dbf,0085cc,b9bbbf"

if [ "$ISDARK" = 1 ]; then
  AGG_THEME="$DARK_THEME"
else
  AGG_THEME="$LGHT_THEME"
fi

agg --cols=80 --rows=25 --font-size=32 --font-family "Fira Mono" --last-frame-duration=5 $AGG_THEME -v demo-$COLTAG.trimmed.cast demo-$COLTAG.gif

gifsicle --lossy=95 -k 128 -O2 -Okeep-empty demo-$COLTAG.gif -o demo-opt-$COLTAG.gif
#gif2webp -min_size -m 6 demo.gif -o demo.webp
