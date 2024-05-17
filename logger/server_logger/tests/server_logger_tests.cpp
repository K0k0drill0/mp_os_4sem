#include <gtest/gtest.h>

#include <iostream>

#include <server_logger.h>

int main(
    int argc,
    char *argv[])
{
    try {

        std::string configuration_file_path("D:\\LABS\\mp_os_4sem\\logger\\server_logger\\tests\\config_example.json");
        std::string configuration_path("server_logger_builder_configuration");

        logger_builder* buider = new server_logger_builder;

        logger *log = buider->transform_with_configuration(configuration_file_path, configuration_path)->build();

        log->debug("omg it works");
    }
    catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    }

    
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}