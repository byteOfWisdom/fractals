#include <cstdint>
#include <complex>
#include <stdio.h>
#include <functional>

#include <raylib.h>
#include <omp.h>


typedef std::complex<double> C;
typedef uint8_t pixel;

pixel calc_frac(int32_t x, int32_t y, double a, double b, int32_t xos, int32_t yos) {
    using namespace std::complex_literals;
    const uint8_t iterations = 255;
    const double threshhold = 355.0;
    //uint8_t amp = 65000 / iterations;
    pixel amp = 1;

    std::complex<double> c = (double) (x + xos) * a + 1i * (double) (y + yos) * b;
    std::complex<double> z = 0.0 + 0.0i;


    for (uint16_t i = 0; i < iterations; i ++) {
        z = z * z + c;
        if (abs(z) > threshhold){
            return i * amp;
        }
    }

    return iterations * amp;
}


pixel calc_julia(int32_t x, int32_t y, double a, double b, int32_t xos, int32_t yos) {
    using namespace std::complex_literals;
    const uint8_t iterations = 127;
    const double threshhold = 100.1;
    const uint8_t amp = 2;

//    auto p = [](C z) {return z * z * z - (1.0 + 0.0i);};
//    auto dp = [](C z) {return (3.0 + 0.0i) * z * z;};


//    auto f = [p, dp](C z) {return z - (p(z) / dp(z));};

    C z = (double) (x + xos) * a + 1i * (double) (y + yos) * b;
    C z_null = z;
    C c = 0.37 + 0.16i;

    for (uint16_t i = 0; i < iterations; i ++) {
//        z = f(z);
        z = z - ((z * z * z + c) / ((3.0 + 0.0i) * z * z));
        if (abs(z - z_null) > threshhold) return i * amp;
    }

    return iterations * amp;
}


int main(void)
{
    const uint16_t res_x = 1080;
    const uint16_t res_y = 720;

    SetTraceLogLevel(LOG_NONE);
    InitWindow(res_x, res_y, "fractals");
    SetTargetFPS(60);

    pixel* data = new pixel[res_x * res_y];

    Image img = Image{
        .data = data,
        .width = res_x,
        .height = res_y,
        .mipmaps = 1,
        //.format = PIXELFORMAT_UNCOMPRESSED_R5G6B5,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
    };
    Texture2D texture = LoadTextureFromImage(img);
    bool changed = true;

    double delta = 0.01;
    double dx = 0;
    double dy = 0;


    while (!WindowShouldClose()) {
        if (changed) {
            changed = false;

            #pragma omp parallel for
            for (uint64_t i = 0; i < res_x * res_y; i ++) {
                data[i] = calc_frac(
                    ( i - i % res_x) / res_x, i % res_x, 
                    delta, delta, 
                    -(res_x / 2) + (int32_t) dx, -(res_y / 2) + (int32_t) dy
                );
            }

            Image img = Image{
                .data = data,
                .width = res_x,
                .height = res_y,
                .mipmaps = 1,
                //.format = PIXELFORMAT_UNCOMPRESSED_R5G6B5,
                .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE,
            };
            UnloadTexture(texture);
            texture = LoadTextureFromImage(img);
        }


        if (IsMouseButtonDown(0)) {
            auto xy = GetMouseDelta();
            dx -= xy.y;
            dy -= xy.x;
            changed = true;
        }

        if (IsKeyDown(KEY_A)) {
            delta *= 0.85;
            dx = dx / 0.85;
            dy = dy / 0.85;
            changed = true;
        }

        if (IsKeyDown(KEY_S)) {
            delta = delta / 0.85;
            dx = dx * 0.85;
            dy = dy * 0.85;
            changed = true;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(texture, 0, 0, RAYWHITE);
        EndDrawing();
    }

    delete[] data;

    return 0;
}
