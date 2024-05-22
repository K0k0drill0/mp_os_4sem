#include <fstream>

#include <server_logger_builder.h>
#include <server_logger.h>

#include <gtest/gtest.h>

int main(
    int argc,
    char *argv[])
{
    //testing::InitGoogleTest(&argc, argv);

    //return RUN_ALL_TESTS();
    
    std::string path = "aboba";
    
    #ifdef _WIN32
    path = "tmp_none";
    #endif
    
    #ifdef __linux__
    path = "/home/kokodrillo/git_projects/mp_os_4sem/logger/server_logger/tests/output.txt";
    #endif
    
    logger_builder *builder = new server_logger_builder();
    
    logger* logger = builder
            ->add_console_stream(logger::severity::information)
            ->add_file_stream(path, logger::severity::information)
            ->add_file_stream(path, logger::severity::trace)
            ->add_file_stream("/", logger::severity::information)
            ->build();
    
    logger->information("INFO!!!");
    logger->trace("TRACE!!!");
    
    delete builder;
    delete logger;
}