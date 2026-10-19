// Writes the Halloween theme (halloween.json) with the Facet theme SDK. The
// module is only this file's output and a manifest: no program runs on the
// device.
//
//   halloween-theme <output.json>
//
// The screen is about 480 dp high and 853 dp wide on 16:9 displays whatever
// the pixel size (dp follow the height); drawings are placed from the edges
// (Num::width / Num::height) so that other shapes of screen work too.
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "facet/theme.h"

using namespace facet::sdk;

namespace {

// Deterministic pseudo-random numbers in [0, 1): the same sky every build.
struct Rng {
    unsigned s;
    explicit Rng(unsigned seed) : s(seed) {}
    double next() {
        s = s * 1664525u + 1013904223u;
        return double((s >> 8) & 0xFFFFFF) / 16777216.0;
    }
    double range(double a, double b) { return a + (b - a) * next(); }
};

constexpr const char* kOrange = "#FF8A1F";
constexpr const char* kOrangeDark = "#D9620B";
constexpr const char* kOrangeMid = "#F07A12";
constexpr const char* kOrangeLight = "#FF9A2E";
constexpr const char* kFace = "#FFD25A";

// A jack-o'-lantern, `size` dp wide, its face lit by a flickering candle.
void pumpkin(Scene& s, Num cx, Num cy, double size, double phase, bool halo = true) {
    const double S = size, ry = 0.40 * size;
    if (halo) s.glow(cx, cy, 1.05 * S, kOrange).alpha(Num::wave(Wave::Flicker, 0.22, 0.4, 1.7, phase));
    s.ellipse(cx - 0.24 * S, cy, 0.27 * S, ry, kOrangeDark);
    s.ellipse(cx + 0.24 * S, cy, 0.27 * S, ry, kOrangeDark);
    s.ellipse(cx - 0.12 * S, cy, 0.27 * S, ry * 1.02, kOrangeMid);
    s.ellipse(cx + 0.12 * S, cy, 0.27 * S, ry * 1.02, kOrangeMid);
    s.ellipse(cx, cy, 0.24 * S, ry * 1.04, kOrangeLight);
    // The stem, bent a little.
    s.poly({cx - 0.05 * S, cy - ry + 0.02 * S, cx + 0.05 * S, cy - ry + 0.02 * S, cx + 0.09 * S, cy - ry - 0.15 * S,
            cx + 0.01 * S, cy - ry - 0.17 * S},
           "#56772A");
    // The face: lit from inside.
    Num glow = Num::wave(Wave::Flicker, 0.7, 1.0, 1.3, phase);
    s.poly({cx - 0.22 * S, cy - 0.05 * S, cx - 0.07 * S, cy - 0.05 * S, cx - 0.145 * S, cy - 0.2 * S}, kFace)
        .alpha(glow);
    s.poly({cx + 0.07 * S, cy - 0.05 * S, cx + 0.22 * S, cy - 0.05 * S, cx + 0.145 * S, cy - 0.2 * S}, kFace)
        .alpha(glow);
    s.poly({cx - 0.26 * S, cy + 0.07 * S, cx - 0.17 * S, cy + 0.04 * S, cx - 0.1 * S, cy + 0.14 * S, cx - 0.03 * S,
            cy + 0.05 * S, cx + 0.04 * S, cy + 0.14 * S, cx + 0.11 * S, cy + 0.04 * S, cx + 0.18 * S, cy + 0.14 * S,
            cx + 0.26 * S, cy + 0.07 * S, cx + 0.2 * S, cy + 0.25 * S, cx + 0.1 * S, cy + 0.2 * S, cx, cy + 0.27 * S,
            cx - 0.1 * S, cy + 0.2 * S, cx - 0.2 * S, cy + 0.25 * S},
           kFace)
        .alpha(glow);
}

// A bat whose wings beat; it flies across from the left to the right.
void bat(Scene& s, Num x, Num y, double w, double beat_period, double phase, const char* color) {
    Num flap = Num::wave(Wave::Triangle, -0.16 * w, 0.16 * w, beat_period, phase);
    auto wing = [&](double side) {
        // Points of one wing, from the body outwards and back along its scalloped edge.
        return std::vector<Num>{x + side * 0.05 * w, y - 0.04 * w,
                                x + side * 0.26 * w, y - 0.2 * w + flap,
                                x + side * 0.5 * w,  y - 0.12 * w + flap + flap,
                                x + side * 0.42 * w, y + 0.0 * w + flap,
                                x + side * 0.33 * w, y + 0.1 * w + flap * 0.5,
                                x + side * 0.24 * w, y + 0.02 * w,
                                x + side * 0.15 * w, y + 0.11 * w,
                                x + side * 0.07 * w, y + 0.06 * w};
    };
    s.poly(wing(-1), color);
    s.poly(wing(1), color);
    s.ellipse(x, y + 0.02 * w, 0.08 * w, 0.14 * w, color);
    s.poly({x - 0.07 * w, y - 0.08 * w, x - 0.045 * w, y - 0.22 * w, x - 0.01 * w, y - 0.1 * w}, color);
    s.poly({x + 0.07 * w, y - 0.08 * w, x + 0.045 * w, y - 0.22 * w, x + 0.01 * w, y - 0.1 * w}, color);
}

// A bat flying right across the screen once in `period` seconds.
void flying_bat(Scene& s, double y, double bob, double w, double period, double phase) {
    Num x = Num::wave(Wave::Saw, -90, 1050, period, phase);
    Num yy = Num(y) + Num::wave(Wave::Sine, -bob, bob, period / 5.0, phase * 3);
    bat(s, x, yy, w, 0.42, phase * 7, "#050208");
}

// A ghost drifting about.
void ghost(Scene& s, Num x, Num y, double w, double phase) {
    Num dx = Num::wave(Wave::Sine, -12, 12, 6.0, phase);
    Num dy = Num::wave(Wave::Sine, -7, 7, 3.6, phase + 0.3);
    Num cx = x + dx, cy = y + dy;
    s.glow(cx, cy + 0.1 * w, 1.0 * w, "#B7A2FF").alpha(0.35);
    std::vector<Num> body;
    // Round head, straight sides, a hem of three scallops.
    for (int i = 0; i <= 12; ++i) {
        double a = 3.14159265 * (1.0 + double(i) / 12.0);  // pi .. 2pi: the upper half
        body.push_back(cx + 0.5 * w * std::cos(a));
        body.push_back(cy + 0.5 * w * std::sin(a));
    }
    body.push_back(cx + 0.5 * w);
    body.push_back(cy + 0.62 * w);
    for (int i = 0; i < 6; ++i) {  // scallops from the right to the left
        double fx = 0.5 - double(i) / 6.0;
        body.push_back(cx + fx * w);
        body.push_back(cy + (i % 2 == 0 ? 0.78 : 0.62) * w);
    }
    body.push_back(cx - 0.5 * w);
    body.push_back(cy + 0.62 * w);
    s.poly(body, "#F2EEFF").alpha(0.88);
    s.ellipse(cx - 0.17 * w, cy - 0.04 * w, 0.075 * w, 0.1 * w, "#2A1744");
    s.ellipse(cx + 0.17 * w, cy - 0.04 * w, 0.075 * w, 0.1 * w, "#2A1744");
    s.ellipse(cx, cy + 0.2 * w, 0.07 * w, 0.11 * w, "#2A1744");
}

// A web in a corner: `sx`, `sy` are 1 or -1 (which corner it hangs in).
void cobweb(Scene& s, Num x, Num y, double size, double sx, double sy, const char* color, double alpha) {
    const int rays = 5;
    for (int i = 0; i <= rays; ++i) {
        double a = 1.5707963 * double(i) / double(rays);
        s.line(x, y, x + sx * size * std::cos(a), y + sy * size * std::sin(a), 1.0, color).alpha(alpha);
    }
    for (int ring = 1; ring <= 4; ++ring) {
        double r = size * double(ring) / 4.0;
        std::vector<Num> pts;
        for (int i = 0; i <= rays; ++i) {
            double a = 1.5707963 * double(i) / double(rays);
            double rr = r * (i == 0 || i == rays ? 1.0 : 0.86);  // sags between the rays
            pts.push_back(x + sx * rr * std::cos(a));
            pts.push_back(y + sy * rr * std::sin(a));
        }
        s.polyline(pts, 0.9, color).alpha(alpha);
    }
}

void sky(Scene& s) {
    // Night: black at the top, a purple haze towards the ground.
    s.grad(0, 0, Num::width(), Num::height(0.55, 2), 0, "#06030D", "#150A27");
    s.grad(0, Num::height(0.55), Num::width(), Num::height(0.45, 1), 0, "#150A27", "#2A1038");

    Rng rng(20261031);
    for (int i = 0; i < 34; ++i) {
        double fx = rng.range(0.01, 0.99), y = rng.range(6, 250), r = rng.range(0.6, 1.7);
        double period = rng.range(2.2, 5.5), phase = rng.next();
        s.circle(Num::width(fx), y, r, "#FFF3D2").alpha(Num::wave(Wave::Sine, 0.25, 0.95, period, phase));
    }

    // The moon, breathing a little.
    Num mx = Num::width(0.58), my = 74;
    s.glow(mx, my, 170, "#FFC873").alpha(Num::wave(Wave::Sine, 0.2, 0.34, 7.0));
    s.circle(mx, my, 46, "#FFE9B8");
    s.circle(mx + 14, my - 10, 9, "#F1D399").alpha(0.8);
    s.circle(mx - 15, my + 14, 6.5, "#F1D399").alpha(0.8);
    s.circle(mx + 6, my + 22, 4, "#F1D399").alpha(0.7);
    s.circle(mx - 20, my - 14, 4.5, "#F1D399").alpha(0.6);

    // Clouds in front of it, drifting.
    for (int i = 0; i < 3; ++i) {
        double y = 60 + 36.0 * i, w = 150 + 40.0 * i, period = 70 + 25.0 * i, phase = 0.27 * i;
        Num x = Num::wave(Wave::Saw, -w, 960, period, phase);
        s.ellipse(x, y, w * 0.5, 13 + 3.0 * i, "#1F0E33").alpha(0.55);
        s.ellipse(x + w * 0.18, y - 7, w * 0.3, 11 + 2.0 * i, "#1F0E33").alpha(0.55);
    }
}

void ground(Scene& s) {
    // Far hills, near hills.
    auto hills = [&](double base, double amp, double shift, const char* color) {
        std::vector<Num> pts;
        pts.push_back(Num::width(0));
        pts.push_back(Num::height(1, 1));
        const int n = 14;
        for (int i = 0; i <= n; ++i) {
            double f = double(i) / n;
            pts.push_back(Num::width(f));
            pts.push_back(Num::height(1, -(base + amp * std::sin(f * 7.0 + shift) + amp * 0.5 * std::sin(f * 17.0))));
        }
        pts.push_back(Num::width(1));
        pts.push_back(Num::height(1, 1));
        s.poly(pts, color);
    };
    hills(46, 11, 0.4, "#150A22");
    hills(26, 8, 2.0, "#0A0512");

    // Fog on the ground.
    s.grad(0, Num::height(1, -96), Num::width(), 96, 0, "#6B3FA000", "#6B3FA040");
    for (int i = 0; i < 2; ++i)
        s.ellipse(Num::wave(Wave::Saw, -200, 1000, 60 + 22.0 * i, 0.4 * i), Num::height(1, -24 - 12.0 * i), 160, 14,
                  "#9A6FE0")
            .alpha(0.1);

    // A dead tree on the left.
    Num tx = Num::width(0.06);
    Num base = Num::height(1);
    s.poly({tx - 5, base - 20, tx + 5, base - 20, tx + 3, base - 96, tx - 3, base - 96}, "#06030B");
    s.line(tx, base - 90, tx - 34, base - 128, 4.0, "#06030B");
    s.line(tx - 34, base - 128, tx - 52, base - 126, 2.4, "#06030B");
    s.line(tx - 34, base - 128, tx - 42, base - 150, 2.2, "#06030B");
    s.line(tx, base - 84, tx + 30, base - 118, 3.6, "#06030B");
    s.line(tx + 30, base - 118, tx + 50, base - 114, 2.2, "#06030B");
    s.line(tx + 30, base - 118, tx + 36, base - 142, 2.0, "#06030B");
    s.line(tx + 2, base - 70, tx + 20, base - 80, 2.6, "#06030B");

    // Tombstones on the right.
    auto tomb = [&](double f, double w, double h, double lift) {
        Num x = Num::width(f);
        Num top = Num::height(1, -(h + lift));
        s.rrect(x, top, w, h, w * 0.5, "#0C0616");
        s.rect(x, top + h * 0.55, w, h * 0.5, "#0C0616");
        s.line(x + w * 0.5, top + h * 0.18, x + w * 0.5, top + h * 0.5, 1.6, "#2A1740");
        s.line(x + w * 0.32, top + h * 0.3, x + w * 0.68, top + h * 0.3, 1.6, "#2A1740");
    };
    tomb(0.77, 26, 38, 8);
    tomb(0.83, 22, 32, 14);
    tomb(0.89, 28, 42, 6);

    flying_bat(s, 120, 16, 46, 21, 0.0);
    flying_bat(s, 190, 22, 34, 29, 0.45);
    flying_bat(s, 70, 12, 28, 37, 0.72);
}

void tile_skin(ThemeDef& t) {
    Scene& u = t.tile(Layer::Under);
    const double r = 22;
    // A glow in the tile's own colour, a neon edge, a body that darkens downwards.
    u.rglow(0, 0, Num::width(), Num::height(), r, 12, "tone").alpha(0.3);
    u.rrect(0, 0, Num::width(), Num::height(), r, "tone").alpha(0.42);
    u.grad(1.6, 1.6, Num::width(1, -3.2), Num::height(1, -3.2), r - 1.6, "#2E1A4C", "#170B28");
    // The press feedback: the card colour lightens a little while a finger is on it.
    u.rrect(1.6, 1.6, Num::width(1, -3.2), Num::height(1, -3.2), r - 1.6, "card").alpha(0.4);

    Scene& o = t.tile(Layer::Over);
    // A web in the top left corner, a small pumpkin peeking over the top right.
    cobweb(o, 3, 3, 30, 1, 1, "#E8DCFF", 0.22);
    pumpkin(o, Num::width(1, -22), 4, 34, 0.3, false);
}

void card_skin(ThemeDef& t) {
    Scene& u = t.card(Layer::Under);
    const double r = 20;
    u.rglow(0, 0, Num::width(), Num::height(), r, 12, kOrange).alpha(0.22);
    u.grad(0, 0, Num::width(), Num::height(), r, "#241539", "#170C29");
    u.rrect(16, 0, Num::width(1, -32), 2, 1, kOrange).alpha(0.6);

    Scene& o = t.card(Layer::Over);
    cobweb(o, Num::width(1, -2), 2, 26, -1, 1, "#E8DCFF", 0.2);
}

void screen_decor(ThemeDef& t) {
    Scene& under = t.screen("*", Layer::Under);
    sky(under);
    ground(under);
    ghost(under, Num::width(0.41), 118, 34, 0.2);
    // Webs in the top corners.
    cobweb(under, 0, 0, 74, 1, 1, "#CDBBF5", 0.2);
    cobweb(under, Num::width(1), 0, 58, -1, 1, "#CDBBF5", 0.16);

    Scene& over = t.screen("*", Layer::Over);
    // Pumpkins in the bottom corners, in the margins beside the lists.
    pumpkin(over, 30, Num::height(1, -34), 54, 0.0);
    pumpkin(over, 70, Num::height(1, -22), 34, 0.5);
    pumpkin(over, Num::width(1, -28), Num::height(1, -26), 40, 0.8);
    (void)0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <output.json>\n", argv[0]);
        return 2;
    }
    ThemeDef t("halloween");
    t.name("en", "Halloween");  // other languages: the manifest
    t.dark(true).animated(true).radius(22);
    t.color("bg", "#0B0614")
        .color("surface", "#1E1230")
        .color("surface_pressed", "#3A2160")
        .color("text", "#F6ECFF")
        .color("text_dim", "#B49BD6")
        .color("accent", kOrange)
        .color("on_accent", "#1A0B00")
        .color("divider", "#36224F")
        .color("track", "#47316B")
        .color("good", "#86E26B")
        .color("warn", "#FFC24A")
        .color("bad", "#FF5C4D")
        .color("scrim", "#05020A99");
    screen_decor(t);
    tile_skin(t);
    card_skin(t);

    std::string error;
    if (!t.save(argv[1], &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    std::printf("wrote %s\n", argv[1]);
    return 0;
}
