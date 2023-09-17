#include <unity.h>
#include <tempr_format.h>

void setUp(void)
{

}

void tearDown(void)
{

}

void test_positive_values()
{
    char buffer[TEMPER_FORMAT_SIZE];

    tempr_format(10035, buffer);
    TEST_ASSERT_EQUAL_STRING("100.35", buffer);

    tempr_format(7042, buffer);
    TEST_ASSERT_EQUAL_STRING("70.42", buffer);

    tempr_format(999, buffer);
    TEST_ASSERT_EQUAL_STRING("9.99", buffer);

    tempr_format(42, buffer);
    TEST_ASSERT_EQUAL_STRING("0.42", buffer);
}

void test_negative_values()
{
    char buffer[TEMPER_FORMAT_SIZE];

    tempr_format(-10035, buffer);
    TEST_ASSERT_EQUAL_STRING("-100.35", buffer);

    tempr_format(-7042, buffer);
    TEST_ASSERT_EQUAL_STRING("-70.42", buffer);

    tempr_format(-999, buffer);
    TEST_ASSERT_EQUAL_STRING("-9.99", buffer);

    tempr_format(-42, buffer);
    TEST_ASSERT_EQUAL_STRING("-0.42", buffer);
}

void test_zero()
{
    char buffer[TEMPER_FORMAT_SIZE];
    tempr_format(0, buffer);
    TEST_ASSERT_EQUAL_STRING("0.00", buffer);
}

void test_robustness()
{
    char buffer[TEMPER_FORMAT_SIZE];
    tempr_format(0, NULL);

    tempr_format(100000, buffer);
    TEST_ASSERT_EQUAL_STRING("-.--", buffer);

    tempr_format(-100000, buffer);
    TEST_ASSERT_EQUAL_STRING("-.--", buffer);
}

// TODO - It seems that the tempr_format needs to be moved to the lib folder...
void app_main()
{
  UNITY_BEGIN();

  RUN_TEST(test_positive_values);
  RUN_TEST(test_negative_values);
  RUN_TEST(test_zero);
  RUN_TEST(test_robustness);

  UNITY_END();
}
