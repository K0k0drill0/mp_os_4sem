#include <gtest/gtest.h>

#include <client_logger.h>

int main(
    int argc,
    char *argv[])
{

    try
    {

    std::string conf_filename = "~/git_projects/mp_os_4sem/build/logger/client_logger/tests/config_example.json";
    std::string conf_path = "real_path:client_logger_builder.config";

    logger_builder *p_obj = new client_logger_builder;

    logger *log = p_obj->transform_with_configuration(conf_filename, conf_path)->build();

    log->information("LOOL");
    log->debug("KEEK");

    delete p_obj;
    delete log;

    }
    catch (std::runtime_error er){
        std::cout << er.what() << std::endl;
    }



    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}