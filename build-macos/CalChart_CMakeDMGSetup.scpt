on run argv
  set image_name to item 1 of argv

  tell application "Finder"
  tell disk image_name

    -- wait for the image to finish mounting
    set open_attempts to 0
    repeat while open_attempts < 4
      try
        open
          delay 1
          set open_attempts to 5
        close
      on error errStr number errorNumber
        set open_attempts to open_attempts + 1
        delay 10
      end try
    end repeat

    -- open the image the first time and save a DS_Store with just
    -- background and icon setup
    open
      set current view of container window to icon view
      set theViewOptions to the icon view options of container window
      set arrangement of theViewOptions to not arranged
      set background picture of theViewOptions to file ".background:background.png"
      set icon size of theViewOptions to 100
    close

    -- next setup the position of the app and Applications symlink
    -- plus hide all the window decoration
    open
      update without registering applications
      tell container window
        set sidebar width to 0
        set statusbar visible to false
        set toolbar visible to false
        set the bounds to { 400, 100, 900, 465 }
        set position of item "CalChart.app" to { 133, 150 }
        set position of item "Applications" to { 378, 140 }
      end tell
      update without registering applications
    close

    -- one last open and close so you can see everything looks correct
    open
      delay 5
    close

  end tell
end tell
end run
