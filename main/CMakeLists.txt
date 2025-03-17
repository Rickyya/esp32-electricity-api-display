idf_component_register(SRCS "main.c" 
                        "internal/display_controller.c" 
                        "internal/time_date.c"
                        "internal/api_controller.c"
                        "internal/button_controller.c"
                    REQUIRES networking esp_wifi nvs_flash ssd1306 driver json lwip esp_netif driver
                    INCLUDE_DIRS ".")