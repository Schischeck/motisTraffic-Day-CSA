# Run

    elm-make Main.elm --output elm.js && static

# Automatic recompilation (requires inotify-tools)

    while inotifywait -r -e close_write Widgets Main.elm; do elm-make Main.elm --output elm.js; done