{
  "targets": [
    {
      "target_name": "nylonfilter",
      "defines": [
        "V8PP_ISOLATE_DATA_SLOT=2",
      ],
      "sources": [
        "main.cpp",
        "buffer.cpp",
        "large_buffer.cpp",
        "layer.cpp",
        "item_value.cpp",
        "session.cpp",
        "packet.cpp",
        "virtual_packet.cpp",
        "packet_store.cpp",
        "stream_chunk.cpp",
        "packet_queue.cpp",
        "nylon_context.cpp",
        "dissector.cpp",
        "dissector_thread.cpp",
        "stream_dissector_thread.cpp",
        "filter_thread.cpp",
        "stream_dispatcher.cpp"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "<!(node -e \"require('v8pp')\")"
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions', '-fno-rtti' ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.9',
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'GCC_ENABLE_CPP_RTTI': 'YES'
          }
        }]
      ]
    }
  ]
}
