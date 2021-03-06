set(CMAKE_CXX_STANDARD 17)
add_executable(switch
    main.cpp
    usbserver.cpp
    usbserver.h
    usbacceptor.h
    usbacceptor.cpp
    usbdevice.cpp
    usbdevice.h
    usbcontext.h
    log.h
    log.cpp
    libusb_utils.h
    error.cpp
    error.h
    aseba_message_parser.h
    aseba_description_receiver.h
    aseba_message_writer.h
    aseba_node_registery.h
    aseba_node_registery.cpp
    aseba_endpoint.h
    aseba_node.h
    aseba_node.cpp
    dispatcher.h
    app_server.h
    app_endpoint.h
    flatbuffers_message_reader.h
    flatbuffers_message_writer.h
    flatbuffers_messages.h
    node_id.h
)
target_link_libraries(switch Boost::program_options Boost::asio Boost::date_time libusb spdlog tl_expected asebacommon asebacompiler aware thymio-flatbuffers)
if(WIN32)
    target_link_libraries(switch bcrypt)
endif()

install(TARGETS switch RUNTIME DESTINATION bin)
codesign(switch)
