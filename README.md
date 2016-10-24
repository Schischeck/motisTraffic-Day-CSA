# Run

    elm-make src/Main.elm --output elm.js && static

# Automatic recompilation (requires inotify-tools or fswatch)

    while inotifywait -r -e close_write src; do elm-make src/Main.elm --output elm.js; done

    fswatch -0 -or src | xargs -0 -n 1 -I {} elm-make src/Main.elm --output elm.js

## Automatic recompilation + live reload (requires elm-live)

    elm-live src/Main.elm --output elm.js
    
