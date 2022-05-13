#include "rtweekend.h"

#include "colour.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "moving_sphere.h"

#include <iostream>
#include <future>
#include <thread>

colour ray_colour(const ray& r, const hittable& world, int depth) {
    hit_record rec;

    if (depth <= 0)
        return colour(0, 0, 0);

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        colour attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_colour(scattered, world, depth - 1);
        return colour(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * colour(1.0, 1.0, 1.0) + t * colour(0.5, 0.7, 1.0);
}

hittable_list two_spheres() {
    hittable_list objects;

    auto checker = make_shared<checker_texture>(colour(0.2, 0.3, 0.1), colour(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list random_scene()
{
    hittable_list world;

    auto checker = make_shared<checker_texture>(colour(0.2, 0.3, 0.1), colour(0.9, 0.9, 0.9));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 centre(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((centre - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = colour::random() * colour::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto centre2 = centre + vec3(0, random_double(0, .5), 0);
                    world.add(make_shared<moving_sphere>(centre, centre2, 0.0, 1.0, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = colour::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(centre, 0.2, sphere_material));
                }
                else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(centre, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(colour(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(colour(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

// Mutex to prevent pixels being written and console log output at the same time
static std::mutex output_mutex;

// Calulate a row of pixels
static void calculate_pixels(
    std::vector<std::vector<std::tuple<int, int, colour>>>& rows, int image_width, int image_height,
    int samples_per_pixel, hittable_list world, camera cam, int max_depth, int j
)
{
    std::vector<std::tuple<int, int, colour>> current_row;
    for (int i = 0; i < image_width; ++i)
    {
        colour pixel_colour(0, 0, 0);
        for (int s = 0; s < samples_per_pixel; ++s)
        {
            auto u = ((i + random_double()) / (image_width - 1));
            auto v = ((j + random_double()) / (image_height - 1));
            ray r = cam.get_ray(u, v);
            pixel_colour += ray_colour(r, world, max_depth);
        }
        current_row.push_back(std::make_tuple(j, i, pixel_colour));
    }
    std::lock_guard<std::mutex> lock(output_mutex);
    rows.push_back(current_row);
    std::cerr << "Scanline: " << j << std::flush;
}

// Wakes the conditional variable when we have finished calculating all rows
static void wake_main_thread(std::vector<std::vector<std::tuple<int, int, colour>>>& rows, std::condition_variable& cv, int image_height)
{
    using namespace std::chrono_literals;
    while (rows.size() != image_height)
    {
        std::this_thread::sleep_for(1s);
    }

    cv.notify_all();
}

int main()
{
    // Image
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 800;
    const int samples_per_pixel = 100;
    const int max_depth = 50;

    // World
    hittable_list world;

    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;

    switch (1) {
    case 1:
        world = random_scene();
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        break;

    default:
    case 2:
        world = two_spheres();
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;
    }

    // Camera

    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    int image_height = static_cast<int>(image_width / aspect_ratio);

    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    // Render
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    std::vector<std::vector<std::tuple<int, int, colour>>> rows;

    std::vector<std::future<void>> future;
    std::mutex threads;
    std::condition_variable cv;

    // Async call all rows to be calculated
    for (int j = image_height - 1; j >= 0; --j)
    {
        future.push_back(std::async(std::launch::async, std::ref(calculate_pixels),
            std::ref(rows), image_width, image_height, samples_per_pixel, world, cam, max_depth, j)
        );
    }
    // Add the wake main thread and wait until it does
    future.push_back(std::async(std::launch::async, std::ref(wake_main_thread), std::ref(rows), std::ref(cv), image_height));
    std::unique_lock<std::mutex> lck(threads);
    cv.wait(lck);

    // Sort the calculated rows based on j coord
    std::sort(rows.begin(), rows.end(), [](const std::vector<std::tuple<int, int, colour>>& a, const std::vector<std::tuple<int, int, colour>>& b) {
        return std::get<0>(a.front()) < std::get<0>(b.front());
    });

    // Write the pixels to the output file
    for (int j = image_height - 1; j >= 0; --j)
    {
        for (int i = 0; i < image_width; i++)
        {
            write_colour(std::cout, std::get<2>(rows.at(j).at(i)), samples_per_pixel);
        }
    }

    std::cerr << "\nDone.\n";
}