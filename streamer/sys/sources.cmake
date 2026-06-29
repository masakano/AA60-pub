
set(app_sources
  base_app.cpp
  sender_app.cpp
  receiver_app.cpp
  viewer_app.cpp
)

set(headers
  base_app.h
  common.h
  ffmpeg_frame_reader.h
)

set(sources
  ${app_sources}
  ${headers}
  main.cpp 
)

