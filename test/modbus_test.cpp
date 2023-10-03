#include <unity.h>

extern "C"
{
    void app_main(void);
}


void test_modbus(void) {
    TEST_ASSERT_EQUAL(1,1);
}


void app_main()
{

    UNITY_BEGIN();    // IMPORTANT LINE!
    RUN_TEST(test_modbus);
    UNITY_END();
}