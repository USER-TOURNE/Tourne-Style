// Look pack check: the miniz calls dots.cpp makes, writing a pack to memory and reading it back, without
// Windows' settings or the Studio. Build and run (from the repo, Windhawk's clang++; miniz as C first):
//   clang++ -x c -std=c11 -w -DMINIZ_NO_TIME -c desktop/third_party/miniz/miniz.c -o miniz.o
//   clang++ -std=c++20 -static -DMINIZ_NO_TIME -Idesktop/third_party/miniz tools/desktop/look-pack-test.cpp miniz.o
//           -o look-pack-test.exe && look-pack-test.exe
#ifndef MINIZ_NO_TIME
#define MINIZ_NO_TIME
#endif
#include "miniz.h"

#include <cstdio>
#include <string>

static int g_failed = 0;
#define CHECK(x)                                                    \
    do {                                                            \
        if (!(x)) std::printf("FAIL line %d: %s\n", __LINE__, #x), g_failed++; \
    } while (0)

int main() {
    std::string look = "dots.name = Test look\ntaskbar.height = 38\n";
    std::string plugins = "# plugins\ntourne/notes\n";
    std::string picture(3 << 20, '\0');
    for (size_t i = 0; i < picture.size(); i++) picture[i] = (char)(i * 2654435761u >> 24);

    mz_zip_archive w{};
    CHECK(mz_zip_writer_init_heap(&w, 0, 0));
    CHECK(mz_zip_writer_add_mem(&w, "look.tourne", look.data(), look.size(), MZ_BEST_COMPRESSION));
    CHECK(mz_zip_writer_add_mem(&w, "plugins.txt", plugins.data(), plugins.size(), MZ_BEST_COMPRESSION));
    CHECK(mz_zip_writer_add_mem(&w, "wallpaper.png", picture.data(), picture.size(), MZ_NO_COMPRESSION));
    void* zip = nullptr;
    size_t size = 0;
    CHECK(mz_zip_writer_finalize_heap_archive(&w, &zip, &size));
    mz_zip_writer_end(&w);
    CHECK(zip && size > picture.size());

    mz_zip_archive r{};
    CHECK(mz_zip_reader_init_mem(&r, zip, size, 0));
    auto read = [&](const char* name, std::string* out) {
        int i = mz_zip_reader_locate_file(&r, name, nullptr, 0);
        mz_zip_archive_file_stat stat{};
        if (i < 0 || !mz_zip_reader_file_stat(&r, i, &stat)) return false;
        out->resize((size_t)stat.m_uncomp_size);
        return mz_zip_reader_extract_to_mem(&r, i, out->data(), out->size(), 0) != 0;
    };
    std::string a, b, c, none;
    CHECK(read("look.tourne", &a) && a == look);
    CHECK(read("plugins.txt", &b) && b == plugins);
    CHECK(read("wallpaper.png", &c) && c == picture);
    CHECK(!read("wallpaper.jpg", &none));
    mz_zip_reader_end(&r);
    mz_free(zip);

    // Not a zip: refused, not misread.
    std::string junk = "not a zip at all";
    mz_zip_archive bad{};
    CHECK(!mz_zip_reader_init_mem(&bad, junk.data(), junk.size(), 0));

    std::printf(g_failed ? "%d failed\n" : "all passed\n", g_failed);
    return g_failed != 0;
}
