// NerdMiner FireBeetle 2 ESP32-E Case with Battery
// Battery: 65x35x10mm LiPo pouch (3000mAh)
// Board: DFRobot FireBeetle 2 ESP32-E (60x25.4mm)
// Layout: battery below, board on top, USB-C at front
//
// Orientation: X = long axis (68.2mm outer), Y = short axis (38.2mm outer)
// USB-C is on the X=0 short edge (front)

/* [Board Dimensions] */
board_l = 60;         // along X
board_w = 25.4;       // along Y
board_h = 1.6;
board_clearance = 6;

/* [Battery Dimensions] */
batt_l = 65;          // along X
batt_w = 35;          // along Y
batt_h = 10;

/* [Case Parameters] */
wall = 1.6;
gap = 0.4;
corner_r = 3;

/* [Computed] */
inner_l = max(batt_l, board_l) + 2*gap;  // 65.8
inner_w = max(batt_w, board_w) + 2*gap;  // 35.8

// Z stack: floor | battery | gap | PCB | components | lid
pcb_z = wall + batt_h + gap;             // top of battery + small gap
lid_z = pcb_z + board_h + board_clearance;

outer_l = inner_l + 2*wall;
outer_w = inner_w + 2*wall;
outer_h = lid_z + wall;

// Board centered in case
board_x = wall + (inner_l - board_l) / 2;
board_y = wall + (inner_w - board_w) / 2;

module rounded_box(l, w, h, r) {
    hull() {
        for (x = [r, l-r])
            for (y = [r, w-r])
                translate([x, y, 0])
                    cylinder(h=h, r=r, $fn=32);
    }
}

module base() {
    difference() {
        // Outer shell - open top
        rounded_box(outer_l, outer_w, outer_h, corner_r);

        // Hollow inside
        translate([wall, wall, wall])
            rounded_box(inner_l, inner_w, outer_h, corner_r - wall + gap);

        // USB-C cutout - front short edge (X=0 wall)
        // Centered on the board's Y position
        usb_w = 10;
        usb_h = 4;
        translate([-1, outer_w/2 - usb_w/2, pcb_z - 0.5])
            cube([wall + 2, usb_w, usb_h + 1]);

        // Ventilation slots on long sides (Y=0 and Y=outer_w walls)
        for (i = [0:4]) {
            slot_x = outer_l/2 - 12 + i*5;
            // Y=0 side
            translate([slot_x, -1, pcb_z + board_h + 1])
                cube([2, wall+2, board_clearance - 2]);
            // Y=outer_w side
            translate([slot_x, outer_w - wall - 1, pcb_z + board_h + 1])
                cube([2, wall+2, board_clearance - 2]);
        }

        // Reset button hole - back short edge (X=outer_l wall)
        translate([outer_l - wall - 1, outer_w/2, pcb_z + board_h/2])
            rotate([0, 90, 0])
                cylinder(d=3, h=wall+2, $fn=20);
    }

    // --- Battery retaining ridges on floor ---
    batt_x = wall + (inner_l - batt_l) / 2;
    batt_y = wall + (inner_w - batt_w) / 2;

    // Ridges along the long edges of battery
    for (y = [batt_y - 0.5, batt_y + batt_w - 0.5])
        translate([batt_x + 5, y, wall])
            cube([batt_l - 10, 1, 2]);

    // Ridges along the short edges of battery
    for (x = [batt_x - 0.5, batt_x + batt_l - 0.5])
        translate([x, batt_y + 5, wall])
            cube([1, batt_w - 10, 2]);

    // --- PCB support brackets ---
    // These are L-shaped brackets attached to the long walls (Y=wall and Y=wall+inner_w)
    // They extend from the wall down to the floor, then have a shelf at pcb_z
    // The board is narrower than the battery, so the brackets sit at the
    // board edge which is inboard of the wall by (inner_w - board_w)/2

    bracket_w = 2;      // bracket thickness along Y
    shelf_depth = 1.5;  // how far shelf extends under PCB

    // Two brackets per long wall, positioned along X to support the board
    for (x_pos = [board_x + 8, board_x + board_l - 18]) {
        // Near wall (Y = wall side) - bracket goes from wall to board edge
        // Vertical part: wall face down to shelf height
        translate([x_pos, wall, wall])
            cube([10, board_y - wall, pcb_z - wall]);
        // Shelf part: horizontal ledge at pcb_z for PCB to rest on
        translate([x_pos, board_y - shelf_depth, pcb_z])
            cube([10, shelf_depth, 1]);

        // Far wall (Y = wall+inner_w side)
        translate([x_pos, board_y + board_w, wall])
            cube([10, (wall + inner_w) - (board_y + board_w), pcb_z - wall]);
        // Shelf
        translate([x_pos, board_y + board_w, pcb_z])
            cube([10, shelf_depth, 1]);
    }

    // Front and back short-wall brackets (X direction)
    // These support the short edges of the PCB
    for (y_pos = [board_y + 4, board_y + board_w - 10]) {
        // Front wall (X=wall side)
        translate([wall, y_pos, wall])
            cube([board_x - wall, 6, pcb_z - wall]);
        // Shelf
        translate([board_x - shelf_depth, y_pos, pcb_z])
            cube([shelf_depth, 6, 1]);

        // Back wall (X=wall+inner_l side)
        translate([board_x + board_l, y_pos, wall])
            cube([(wall + inner_l) - (board_x + board_l), 6, pcb_z - wall]);
        // Shelf
        translate([board_x + board_l, y_pos, pcb_z])
            cube([shelf_depth, 6, 1]);
    }
}

module lid() {
    lip_depth = 2;  // how far the lip drops into the case

    difference() {
        union() {
            // Top plate
            rounded_box(outer_l, outer_w, wall, corner_r);

            // Inner lip
            translate([wall + gap, wall + gap, -lip_depth])
                rounded_box(inner_l - 2*gap, inner_w - 2*gap, lip_depth + wall, corner_r - wall);
        }

        // Hollow out the lip so it's just a rim
        translate([wall + gap + 1.2, wall + gap + 1.2, -lip_depth - 1])
            rounded_box(inner_l - 2*gap - 2.4, inner_w - 2*gap - 2.4, lip_depth + wall, corner_r - wall - 1);

        // Ventilation slots on top
        for (i = [0:4])
            translate([outer_l/2 - 12 + i*5, outer_w/2 - 8, -1])
                cube([2, 16, wall + 2]);

        // LED window
        translate([board_x + 30, outer_w/2, -1])
            cylinder(d=3, h=wall + 2, $fn=20);
    }
}

// --- Print layout ---
base();

// Lid next to base, flipped for printing
translate([outer_l + 10, 0, wall])
    rotate([180, 0, 0])
        lid();
