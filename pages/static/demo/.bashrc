custom_cmake() {
  if [ "$1" = "-G" ]; then
    /usr/bin/cmake -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=ON -DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=ON -DCMAKE_DISABLE_FIND_PACKAGE_Readline=ON -DCMAKE_DISABLE_FIND_PACKAGE_Editline=ON "$@"
  else
    /usr/bin/cmake "$@"
  fi
}

slow_cat() {
  while IFS= read -r line; do
    if echo "$line" | grep -E '^.{1,15}    .{1,15}/\*' > /dev/null; then
    sleep 2.0
    else
      sleep 0.1
    fi
    echo "$line"
  done
}

slow_bat() {
  bat -P --color=always --style=grid,header "$@" | slow_cat
}

export PS1='\[\e[34m\]~home: \$\[\e[0m\]'
export PS2='\[\e[34m\]       > \[\e[0m\]'
PROMPT_COMMAND="export PROMPT_COMMAND=echo" # Add a line on 2nd and subsequent lines
alias cat=slow_bat
alias cmake=custom_cmake
