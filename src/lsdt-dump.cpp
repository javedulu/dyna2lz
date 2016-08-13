//
// LS-Dyna Tools (dyna2lz) source code. (C) 2006 Max Lapan <lapan_mv@inbox.ru>
//
// ls-dyna database conversion utility
//
// $Log: lsdt-dump.cpp,v $
// Revision 1.4  2007/06/27 19:02:48  max_lapan
// new revision
//
// Revision 1.3  2006/03/08 22:04:59  max_lapan
// 1. Added option -- keep deleted elements. If on, field 'Deleted' will be created.
// 2. Minor fixes
//
// Revision 1.2  2006/03/03 18:32:27  max_lapan
// A lot of changes. Most valuable:
//
// 1. Finished timesteps state dump. It should work in all cases.
// 2. Fixed bug with element kind determination
// 3. SPH, RigidBody, user materials and all other non-generic things are skipped.
//
// Revision 1.1.1.1  2006/02/24 09:30:18  max_lapan
// initial import
//
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkUnstructuredGrid.h>

#include "d3plot.h"



int main (int argc, char** argv)
{
    PartIDFilter filter;

//     filter.appendValue (6);
//     filter.appendValue (7);
//     filter.appendValue (19);
    
    StateOptions opts (false, false, &filter);

    static char fileName[1024];
    float time = 0.0;
    int index = 0;

    if (argc < 3) {
        printf ("Usage: lsdt-dump d3plot basename\n");
        return 0;
    }

    D3PlotFile f (argv[1]);

    // control information bout all these d3plots
    printf ("Read control information..."); fflush (stdout);
    D3PlotControl ctl (&f);
    printf ("done\n");

    // TODO
    assert (!ctl.mattypes ());

    // fluid materials
    if (ctl.fluid_mats () > 0) {
        printf ("Fluid materials... skipped\n");
        f.skip (ctl.fluid_mats () * 4);
    }

    printf ("Reading initial geometry... "); fflush (stdout);
//    f.sayPos ();
    D3PlotGeometry geo (&f, &ctl, &opts);
    printf ("done\n");
    printf ("\n");
    printf ("==================================================\n");
    printf ("Geometry statistics (ElementType field):\n");
    printf ("--------------------------------------------------\n");
    printf ("Points           : %u\n", geo.getPointsCount ());
    printf ("Lines (3)        : %u\n", geo.getLinesCount ());
    printf ("Triangles (5)    : %u\n", geo.getTrianglesCount ());
    printf ("Quads (9)        : %u\n", geo.getQuadsCount ());
    printf ("Tetras (10)      : %u\n", geo.getTetrasCount ());
    printf ("Pyramids (14)    : %u\n", geo.getPyramidsCount ());
    printf ("Hexahedrons (12) : %u\n", geo.getHexasCount ());
    printf ("Wedges (13)      : %u\n", geo.getWedgesCount ());
    printf ("==================================================\n");
    printf ("\n");

    // user material, node and element identification (TODO)
    if (ctl.narbs () > 0) {
        printf ("User materials... skipped\n");
        f.skip (ctl.narbs () * 4);
    }

    // SPH data (TODO)
    if (ctl.sph_nodes () + ctl.sph_mats () > 0) {
        f.skip (sizeof (int) * (ctl.sph_nodes () + ctl.sph_mats ()));
        printf ("SPH data... skipped\n");
    }

    // Rigid road (TODO)
    if (ctl.road_movement ())
        printf ("Rigid road... skipped\n");

    printf ("Writing VTK file (%s)... ", argv[2]); fflush (stdout);
    if (geo.save (argv[2]))
        printf ("done\n");
    else
        printf ("faield\n");

    printf ("State data...\n");

    try {
        while (1) {
            geo.resetState ();
            D3PlotState state (&opts, &ctl, &geo, &f);

            printf ("t = %.6f... ", state.time ()); fflush (stdout);
            state.read ();
            printf ("done\n");

            // prepare output file name
            printf ("Writing VTK file (%d)...", index); fflush (stdout);
            state.save (argv[2], index);
            printf ("done\n");
            index++;
        }
    }
    catch (int code) {
        switch (code) {
        case 0:
            printf ("End of states reached\n");
            break;
        }
    }

    return 0;
}

