#ifndef _TEST_MACRO_
#define _TEST_MACRO_


#define TEST_START(test_number) \
    const auto __WARNINGS__ = get_warnings_count(); \
    const auto __ERRORS__ = get_errors_count(); \
    HAIER_LOGI("Test #" #test_number);

#define TEST_END(warn, err) \
    if (((__WARNINGS__ + (warn)) != get_warnings_count()) || ((__ERRORS__ + (err)) != get_errors_count())) { \
        HAIER_LOGE("Test failed!"); \
        exit(1); \
    }

#endif