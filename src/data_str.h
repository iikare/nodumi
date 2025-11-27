#pragma once
// clang-format off
#define LABEL_LANG_EN unordered_map<string, string> { \
                        {"FILE_MENU_FILE",                      "File"}, \
                        {"FILE_MENU_OPEN_FILE",                 "Open File"}, \
                        {"FILE_MENU_OPEN_IMAGE",                "Open Image"}, \
                        {"FILE_MENU_SAVE",                      "Save"}, \
                        {"FILE_MENU_SAVE_AS",                   "Save As"}, \
                        {"FILE_MENU_CLOSE_FILE",                "Close File"}, \
                        {"FILE_MENU_CLOSE_IMAGE",               "Close Image"}, \
                        {"FILE_MENU_RELOAD",                    "Reload File"}, \
                        {"FILE_MENU_EXIT",                      "Exit"}, \
                        {"EDIT_MENU_EDIT",                      "Edit"}, \
                        {"EDIT_MENU_ENABLE_SHEET_MUSIC",        "Enable Sheet Music"}, \
                        {"EDIT_MENU_DISABLE_SHEET_MUSIC",       "Disable Sheet Music"}, \
                        {"EDIT_MENU_PREFERENCES",               "Preferences"}, \
                        {"VIEW_MENU_VIEW",                      "View"}, \
                        {"VIEW_MENU_DISPLAY_MODE",              "Display Mode:"}, \
                        {"VIEW_MENU_SHOW_SONG_TIME",            "Show Song Time:"}, \
                        {"VIEW_MENU_HIDE_SONG_TIME",            "Hide Song Time"}, \
                        {"VIEW_MENU_SHOW_KEY_SIGNATURE",        "Show Key Signature"}, \
                        {"VIEW_MENU_HIDE_KEY_SIGNATURE",        "Hide Key Signature"}, \
                        {"VIEW_MENU_SHOW_TEMPO",                "Show Tempo"}, \
                        {"VIEW_MENU_HIDE_TEMPO",                "Hide Tempo"}, \
                        {"VIEW_MENU_HIDE_NOW_LINE",             "Hide Now Line"}, \
                        {"VIEW_MENU_SHOW_NOW_LINE",             "Show Now Line"}, \
                        {"VIEW_MENU_HIDE_MEASURE_LINE",         "Hide Measure Line"}, \
                        {"VIEW_MENU_SHOW_MEASURE_LINE",         "Show Measure Line"}, \
                        {"VIEW_MENU_HIDE_MEASURE_NUMBER",       "Hide Measure Number"}, \
                        {"VIEW_MENU_SHOW_MEASURE_NUMBER",       "Show Measure Number"}, \
                        {"VIEW_MENU_HIDE_BACKGROUND",           "Hide Background"}, \
                        {"VIEW_MENU_SHOW_BACKGROUND",           "Show Background"}, \
                        {"VIEW_MENU_SHOW_FPS",                  "Show FPS"}, \
                        {"VIEW_MENU_HIDE_FPS",                  "Hide FPS"}, \
                        {"DISPLAY_MENU_DEFAULT",                "Bar"}, \
                        {"DISPLAY_MENU_LINE",                   "Line"}, \
                        {"DISPLAY_MENU_PULSE",                  "Pulse"}, \
                        {"DISPLAY_MENU_BALL",                   "Ball"}, \
                        {"DISPLAY_MENU_FFT",                    "FFT"}, \
                        {"DISPLAY_MENU_VORONOI",                "Voronoi"}, \
                        {"DISPLAY_MENU_LOOP",                   "Loop"}, \
                        {"SONG_MENU_RELATIVE",                  "Relative"}, \
                        {"SONG_MENU_ABSOLUTE",                  "Absolute"}, \
                        {"MIDI_MENU_MIDI",                      "Midi"}, \
                        {"MIDI_MENU_INPUT",                     "Input"}, \
                        {"MIDI_MENU_OUTPUT",                    "Output"}, \
                        {"MIDI_MENU_ENABLE_LIVE_PLAY",          "Enable Live Play"}, \
                        {"MIDI_MENU_DISABLE_LIVE_PLAY",         "Disable Live Play"}, \
                        {"COLOR_MENU_COLOR",                    "Color"}, \
                        {"COLOR_MENU_COLOR_BY",                 "Color By:"}, \
                        {"COLOR_MENU_COLOR_SCHEME",             "Color Scheme:"}, \
                        {"COLOR_MENU_SWAP_COLORS",              "Swap Colors"}, \
                        {"COLOR_MENU_CYCLE_COLORS",             "Cycle Colors"}, \
                        {"COLOR_MENU_INVERT_COLOR_SCHEME",      "Invert Color Scheme"}, \
                        {"SCHEME_MENU_PART",                    "Part"}, \
                        {"SCHEME_MENU_VELOCITY",                "Velocity"}, \
                        {"SCHEME_MENU_TONIC",                   "Tonic"}, \
                        {"INFO_MENU_INFO",                      "Info"}, \
                        {"INFO_MENU_PROGRAM_INFO",              "Program Info"}, \
                        {"INFO_MENU_FILE_INFO",                 "File Info"}, \
                        {"INFO_MENU_HELP",                      "Help"}, \
                        {"PALETTE_MENU_DEFAULT",                "Default"}, \
                        {"PALETTE_MENU_FROM_BACKGROUND",        "From Background"}, \
                        {"RIGHT_MENU_INFO",                     "Info"}, \
                        {"RIGHT_MENU_CHANGE_PART_COLOR",        "Change Part Color"}, \
                        {"RIGHT_MENU_CHANGE_LINE_COLOR",        "Change Line Color"}, \
                        {"RIGHT_MENU_CHANGE_COLOR",             "Change Color"}, \
                        {"RIGHT_MENU_SET_TONIC",                "Set Tonic"}, \
                        {"RIGHT_MENU_REMOVE_IMAGE",             "Remove Image"}, \
                        {"RIGHT_MENU_FIT_IMAGE_WIDTH",          "Fit Image Width"}, \
                        {"RIGHT_MENU_FIT_IMAGE_HEIGHT",         "Fit Image Height"}, \
                        {"RIGHT_MENU_EMPTY",                    "Empty"}, \
                        {"COLORSELECT_COLOR_SELECT",            "Color Select"}, \
                        {"GET_SONG_INFO_TRACK",                 "Track"}, \
                        {"GET_LABEL_MAJOR",                     "Major"}, \
                        {"WIN_WARN_MSG_EXPERIMENTAL",           "WARNING: windows version is experimental"}, \
                        {"INFO_BOX_BUILD_FLAGS",                "Flags:"}, \
                        {"INFO_BOX_BUILD_DATE",                 "Build Date:"}, \
                        {"INFO_BOX_VER",                        "Ver."}, \
                        {"INFO_BOX_LICENSE_GPL3",               "Licensed under GPLv3"}, \
                        {"PREF_LABEL",                          "Preferences"}, \
                        {"PREF_P1",                             "Display"}, \
                        {"PREF_P2",                             "Effects"}, \
                        {"PREF_P3",                             "Image"}, \
                        {"PREF_TRACK_DIVIDE_LIVE",              "Adaptive Track Division (live)"}, \
                        {"PREF_TRACK_DIVIDE_MIDI",              "Adaptive Track Division (file)"}, \
                        {"PREF_HAND_RANGE",                     "Custom Hand Range"}, \
                        {"PREF_USE_LSTM",                       "Use LSTM Classifier"}, \
                        {"PREF_IMAGE_DARKEN",                   "Darken Background Image"}, \
                        {"PREF_DYNAMIC_LABEL",                  "Dynamic Label Color"}, \
                        {"PREF_CIE_FUNCTION",                   "Set CIE Distance Function"}, \
                        {"PREF_PARTICLE",                       "Enable Particle Effects"}, \
                        {"PREF_SCALE_VELOCITY",                 "Use Scaled Velocities"}, \
                        {"PREF_SHADOW",                         "Enable Drop Shadow"}, \
                        {"PREF_ANGLE",                          "Shadow Angle"}, \
                        {"PREF_NOW_LINE_USE_OVERLAY",           "Enable Overlay Blend Mode"}, \
                        {"PREF_LEVEL_CONTROL",                  "Enable Level Control (RGB)"}, \
                        {"PREF_LIMIT_FPS",                      "Limit FPS"}, \
                        {"FILE_INFO_LABEL",                     "Information"}, \
                        {"FILE_TYPE",                           "Format"}, \
                        {"FILE_NOTE_COUNT",                     "Note Count"}, \
                        {"FILE_MEASURE_COUNT",                  "Measure Count"}, \
                        {"FILE_TRACK_COUNT",                    "Track Count"}, \
                        {"",                                    ""}, \
                      }
// clang-format on
