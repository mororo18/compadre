#include <utest.h/utest.h>
#include <compadre.hpp>
#include <string>

UTEST(preprocess, portuguese_text) {
    auto text = std::string("ÀÁÂÃÄÅ àáâãäå ÉÊËéêë ÍÎÏíîï ÓÔÕÖóôõö ÚÛÜúûü Çç 1234!@#$%^&*()-_=+[]{}|;:',.<>?/`~   ");
    auto expected = std::string("AAAAAA AAAAAA EEEEEE IIIIII OOOOOOOO UUUUUU CC");
    auto preproc = compadre::preprocess_portuguese_text(text);
    ASSERT_EQ(expected.size(), preproc.size());

    for (auto const [index, num] : std::views::enumerate(expected)) {
        ASSERT_EQ(num, preproc.at(index));
    }
}

UTEST_MAIN()
