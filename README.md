# Run

    elm-make Main.elm --output elm.js && static

# Automatic recompilation (requires inotify-tools or fswatch)

    while inotifywait -r -e close_write Widgets Main.elm; do elm-make Main.elm --output elm.js; done

    fswatch -0 -or Widgets Main.elm | xargs -0 -n 1 -I {} elm-make Main.elm --output elm.js
