idf_component_register(SRCS "display_controller.c"
                    REQUIRES ssd1306 driver api_controller time_utils
                    INCLUDE_DIRS "include")
