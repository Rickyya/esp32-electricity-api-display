idf_component_register(SRCS "main.c" 
                    REQUIRES  nvs_flash driver lwip esp_netif 
                        api_controller display_controller time_utils networking button_controller
                    INCLUDE_DIRS ".")