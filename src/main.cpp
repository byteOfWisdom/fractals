#include <raylib.h>
#include <cstdint>
#include <complex>
#include <stdio.h>
#include <omp.h>


uint8_t calc_frac(int32_t x, int32_t y, double a, double b, int32_t xos, int32_t yos) {
    using namespace std::complex_literals;
    const uint8_t iterations = 255;
    const double threshhold = 255.0;
    uint8_t amp = 1;

    std::complex<double> c = (double) (x + xos) * a + 1i * (double) (y + yos) * b;
    std::complex<double> z = 0.0 + 0.0i;

    z = c;

    for (uint16_t i = 0; i < iterations; i ++) {
        z = z * z + c;
        if (abs(z) > threshhold){
            return i * amp;
        } 
    }

    return iterations * amp;
}


int main(void)
{
    SetTraceLogLevel(LOG_NONE);
    InitWindow(1080, 720, "fractals");
    SetTargetFPS(60);

    uint8_t* data = new uint8_t[1080 * 720];


    Image img = Image{
        .data = data,
        .width = 1080,
        .height = 720,
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

            //printf("%e\n", delta);

            #pragma omp parallel for
            for (uint64_t i = 0; i < 1080 * 720; i ++) {
                data[i] = calc_frac(
                    ( i - i % 1080) / 1080, i % 1080, 
                    delta, delta, 
                    -540 + (int32_t) dx, -360 + (int32_t) dy
                );
            }

            Image img = Image{
                .data = data,
                .width = 1080,
                .height = 720,
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