#include "rtweekend.h"

#include "colour.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "moving_sphere.h"
#include "aarect.h"
#include "box.h"

#include <iostream>
#include <future>
#include <thread>

static colour ray_colour(const ray& r, const colour& background, const hittable& world, int depth) {
    hit_record rec;

    if (depth <= 0)
        return colour(0, 0, 0);

    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    colour attenuation;
    colour emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_colour(scattered, background, world, depth-1);
}

static hittable_list two_spheres() {
    hittable_list objects;

    auto checker = make_shared<checker_texture>(colour(0.2, 0.3, 0.1), colour(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

static hittable_list two_perlin_spheres()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);

    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    return objects;
}

static hittable_list random_scene()
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

static hittable_list earth()
{
    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

static hittable_list simple_light()
{
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(colour(4, 4, 4));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

static hittable_list cornell_box()
{
    hittable_list objects;

    auto red = make_shared<lambertian>(colour(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(colour(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(colour(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(colour(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
    
    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
    box2 = make_shared<rotate_y>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130, 0, 65));
    objects.add(box2);

    return objects;
}

// Mutex to prevent pixels being written and console log output at the same time
static std::mutex output_mutex;

// Calulate a row of pixels
static void calculate_pixels(
    std::vector<std::vector<std::tuple<int, int, colour>>>& rows, const hittable_list& world, const colour& background, camera& cam,
    int image_width, int image_height, int samples_per_pixel, int max_depth, int j
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
            pixel_colour += ray_colour(r, background, world, max_depth);
        }
        current_row.push_back(std::make_tuple(j, i, pixel_colour));
    }
    std::lock_guard<std::mutex> lock(output_mutex);
    rows.push_back(current_row);
    std::cerr << "Scanline: " << j << std::endl;
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
    const double aspect_ratio = 1.0;
    const int image_width = 600;
    const int samples_per_pixel = 200;
    const int max_depth = 50;

    // World
    hittable_list world;

    point3 lookfrom;
    point3 lookat;
    double vfov = 40.0;
    double aperture = 0.0;
    colour background(0, 0, 0);

    switch (0) {
    case 1:
        world = random_scene();
        background = colour(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        aperture = 0.1;
        break;

    case 2:
        world = two_spheres();
        background = colour(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;

    case 3:
        world = two_perlin_spheres();
        background = colour(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;
    
    case 4:
        world = earth();
        background = colour(0.70, 0.80, 1.00);
        lookfrom = point3(13, 2, 3);
        lookat = point3(0, 0, 0);
        vfov = 20.0;
        break;
    
    case 5:
        world = simple_light();
        background = colour(0, 0, 0);
        lookfrom = point3(26, 3, 6);
        lookat = point3(0, 2, 0);
        vfov = 20.0;
        break;
    default:
    case 6:
        world = cornell_box();
        background = colour(0, 0, 0);
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
        vfov = 40.0;
        break;
    }

    // Camera

    vec3 vup(0, 1, 0);
    double dist_to_focus = 10.0;
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
            std::ref(rows), std::ref(world), std::ref(background), std::ref(cam),
            image_width, image_height, samples_per_pixel, max_depth, j)
        );
    }
    // Add the wake main thread and wait until it does
    future.push_back(std::async(std::launch::async, std::ref(wake_main_thread),
        std::ref(rows), std::ref(cv), image_height)
    );
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