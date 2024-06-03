#include <gtest/gtest.h>

#include <big_integer.h>
#include <client_logger.h>
#include <operation_not_supported.h>
#include <version>

logger *create_logger(
    std::vector<std::pair<std::string, logger::severity>> const &output_file_streams_setup,
    bool use_console_stream = true,
    logger::severity console_stream_severity = logger::severity::debug)
{
    logger_builder *builder = new client_logger_builder();
    
    if (use_console_stream)
    {
        builder->add_console_stream(console_stream_severity);
    }
    
    for (auto &output_file_stream_setup: output_file_streams_setup)
    {
        builder->add_file_stream(output_file_stream_setup.first, output_file_stream_setup.second);
    }
    
    logger *built_logger = builder->build();
    
    delete builder;
    
    return built_logger;
}

TEST(positive_tests, test1)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });
    
    big_integer bigint_1("32850346459076457453464575686784654");
    big_integer bigint_2("423534596495087569087908753095322");
    big_integer result_of_sum = bigint_1 + bigint_2;
    
    std::stringstream ss;
    ss << result_of_sum;
    std::string result_string = ss.str();
    
    EXPECT_TRUE(result_string == "33273881055571545022552484439879976");
    
    delete logger;
}

TEST(positive_tests, test2)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });
    
    big_integer bigint_1("32850346459076457453464575686784654");
    big_integer bigint_2("0000042353459649508756908790875309532245366457546765745645647567575");


    big_integer result_of_sub = bigint_1 - bigint_2;

    // big_integer should_be("-42353459649508756908790875276681898907381089312281069960782921");

    std::stringstream ss;
    ss << result_of_sub;
    std::string result_string = ss.str();
    
    EXPECT_TRUE(result_string == "-42353459649508756908790875276681898907381089312281069960782921");
    
    delete logger;
}

TEST(positive_tests, test3)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });

    big_integer bigint_1("32850346459076457453464575686784654");
    big_integer bigint_2("12342357553253");

    big_integer result_of_remainder = bigint_1 % bigint_2;

    std::stringstream ss;
    ss << result_of_remainder;
    std::string result_string = ss.str();

    EXPECT_TRUE(result_string == "3232571319826");

    delete logger;
}

TEST(positive_tests, test4)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });
    
    big_integer bigint_1("-54357893745346457544353");
    big_integer bigint_2("-54357893745346457544354");
    
    EXPECT_TRUE(bigint_1 > bigint_2);
    
    delete logger;
}

TEST(positive_tests, test5)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });
    
    big_integer bigint_1("423895435432");
    big_integer bigint_2("423895435432");
    
    EXPECT_TRUE(bigint_1 >= bigint_2);
    
    delete logger;
}

TEST(positive_tests, test6)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });
    
    big_integer bigint_1("-423895435432312432534645756753");
    big_integer bigint_2("0");
    
    EXPECT_TRUE(bigint_1 < bigint_2);
    
    delete logger;
}

TEST(positive_tests, test7)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });
    
    big_integer bigint_1("4238954354324222200000000");
    big_integer bigint_2("4238954354324222222222234");
    
    EXPECT_TRUE(bigint_1 <= bigint_2);
    
    delete logger;
}

TEST(positive_tests, test8)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });
    
    big_integer bigint_1("4238954354321");
    big_integer bigint_2("423895435432");
    
    EXPECT_TRUE(bigint_1 != bigint_2);
    
    delete logger;
}

TEST(positive_tests, test9)
{
    logger *logger = create_logger(std::vector<std::pair<std::string, logger::severity>>
        {
            {
                "bigint_logs.txt",
                logger::severity::information
            },
        });
    
    big_integer bigint_1("-00000044234235347865897389456748953795739648996453238954354321");
    big_integer bigint_2("-00000044234235347865897389456748953795739648996453238954354321");
    
    EXPECT_TRUE(bigint_1 == bigint_2);
    
    delete logger;
}

void memory_dump(unsigned int num) {
    for (int i = sizeof(unsigned int) - 1; i >= 0; i--) {
        for (int j = 7; j >= 0; j--) 
            std::cout << ((num >> (i * 8 + j)) & 1);
        std::cout << ' ';
        //num >>= 8;
    }
    std::cout << std::endl;
}

int main(
    int argc,
    char **argv)
{

    big_integer st("1");

    for (big_integer i("1"); i <= big_integer("1000"); i+=big_integer("1")) {
        st *= i;
    }

    big_integer expected("402387260077093773543702433923003985719374864210714632543799910429938512398629020592044208486969404800479988610197196058631666872994808558901323829669944590997424504087073759918823627727188732519779505950995276120874975462497043601418278094646496291056393887437886487337119181045825783647849977012476632889835955735432513185323958463075557409114262417474349347553428646576611667797396668820291207379143853719588249808126867838374559731746136085379534524221586593201928090878297308431392844403281231558611036976801357304216168747609675871348312025478589320767169132448426236131412508780208000261683151027341827977704784635868170164365024153691398281264810213092761244896359928705114964975419909342221566832572080821333186116811553615836546984046708975602900950537616475847728421889679646244945160765353408198901385442487984959953319101723355556602139450399736280750137837615307127761926849034352625200015888535147331611702103968175921510907788019393178114194545257223865541461062892187960223838971476088506276862967146674697562911234082439208160153780889893964518263243671616762179168909779911903754031274622289988005195444414282012187361745992642956581746628302955570299024324153181617210465832036786906117260158783520751516284225540265170483304226143974286933061690897968482590125458327168226458066526769958652682272807075781391858178889652208164348344825993266043367660176999612831860788386150279465955131156552036093988180612138558600301435694527224206344631797460594682573103790084024432438465657245014402821885252470935190620929023136493273497565513958720559654228749774011413346962715422845862377387538230483865688976461927383814900140767310446640259899490222221765904339901886018566526485061799702356193897017860040811889729918311021171229845901641921068884387121855646124960798722908519296819372388642614839657382291123125024186649353143970137428531926649875337218940694281434118520158014123344828015051399694290153483077644569099073152433278288269864602789864321139083506217095002597389863554277196742822248757586765752344220207573630569498825087968928162753848863396909959826280956121450994871701244516461260379029309120889086942028510640182154399457156805941872748998094254742173582401063677404595741785160829230135358081840096996372524230560855903700624271243416909004153690105933983835777939410970027753472000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");

    // ans.dump_value(std::cout);

    std::cout << (st - expected == big_integer("0")) << std::endl;

    // testing::InitGoogleTest(&argc, argv);
    
    // return RUN_ALL_TESTS();
}