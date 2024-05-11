#include <gtest/gtest.h>

#include <client_logger.h>

int main(
    int argc,
    char *argv[])
{

    try
    {

    std::string conf_filename = "/home/kokodrillo/git_projects/mp_os_4sem/logger/client_logger/tests/config_example.json";
    std::string conf_path = "real_path:client_logger_builder.config";

    logger_builder *p_obj = new client_logger_builder;

    logger *log = p_obj->transform_with_configuration(conf_filename, conf_path)->build();

    log->information("LOOL");
    log->debug("KEEK");
    log->warning("sme_wrn");

    delete p_obj;
    delete log;

    }
    catch (std::exception er) {
        std::cout << er.what() << std::endl;
    }



    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}