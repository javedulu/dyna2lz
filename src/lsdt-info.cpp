//
// LS-Dyna Tools (dyna2lz) source code. (C) 2006 Max Lapan <lapan_mv@inbox.ru>
//
// $Log: lsdt-info.cpp,v $
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

#include "d3plot.h"



int main (int argc, char** argv)
{
    if (argc < 2) {
	printf ("Usage: lsdt-info d3plot\n");
	return 0;
    }

    D3PlotFile f (argv[1]);

    D3PlotControl ctl (&f);

    // it's time to display something
    printf ("======================================================================\n");
    printf ("Database control information:\n");
    printf ("----------------------------------------------------------------------\n");
    printf ("Model name : %s\n", ctl.model_descr ());
    printf ("Run time   : %08x\n", ctl.runtime ());
    printf ("Run date   : %08x\n", ctl.rundate ());
    printf ("Machine ID : %08x\n", ctl.machine ());
    printf ("Code ID    : %08x\n", ctl.codeid ());
    printf ("Code Ver   : %.2f\n", ctl.codever ());
    printf ("Conns pres?: %d\n", ctl.elem_conns ());
    printf ("Mats pres? : %d\n", ctl.mattypes ());
    printf ("Rigid road?: %d\n", ctl.road_movement ());
    printf ("Nodes      : %d\n", ctl.nodes ());
    printf ("New code?  : %d\n", ctl.code_is_new ());
    printf ("Temps?     : %d\n", ctl.temperatures ());
    printf ("Curr geom? : %d\n", ctl.cur_geom ());
    printf ("Velocities?: %d\n", ctl.velocities ());
    printf ("Accels?    : %d\n", ctl.accelerations ());
    printf ("Glob vars  : %d\n", ctl.num_global_vars ());
    printf ("\n");
    printf ("8-elems    : %d\n", ctl.num_8_node_elems ());
    printf ("8-mats     : %d\n", ctl.num_8_node_mats ());
    printf ("8-vals     : %d\n", ctl.num_8_node_vals ());
    printf ("8-add      : %d\n", ctl.num_8_node_add ());
    printf ("\n");
    printf ("2-elems    : %d\n", ctl.num_2_node_elems ());
    printf ("2-mats     : %d\n", ctl.num_2_node_mats ());
    printf ("2-vals     : %d\n", ctl.num_2_node_vals ());
    printf ("\n");
    printf ("4-elems    : %d\n", ctl.num_4_node_elems ());
    printf ("4-mats     : %d\n", ctl.num_4_node_mats ());
    printf ("4-vals     : %d\n", ctl.num_4_node_vals ());
    printf ("4-add      : %d\n", ctl.num_4_node_add ());
    printf ("4-int pts. : %d\n", ctl.num_4_node_int ());
    printf ("\n");
    printf ("SPH Nodes  : %d\n", ctl.sph_nodes ());
    printf ("SPH Mats   : %d\n", ctl.sph_mats ());
    printf ("\n");
    printf ("TH-elems   : %d\n", ctl.thick_shell_elems ());
    printf ("TH-mats    : %d\n", ctl.thick_shell_mats ());
    printf ("TH-vals    : %d\n", ctl.thick_shell_vals ());
    printf ("\n");
    printf ("Deletion   : %d\n", ctl.elems_deletion ());
    printf ("Narbs      : %d\n", ctl.narbs ());
    printf ("Stress?    : %d\n", ctl.stress_components ());
    printf ("Plastic?   : %d\n", ctl.plastic_strain ());
    printf ("Sh force?  : %d\n", ctl.shell_force_res ());
    printf ("Sh energy? : %d\n", ctl.shell_thickness_energy ());
    printf ("Fluid mats : %d\n", ctl.fluid_mats ());
    printf ("CFD flags1 : %d\n", ctl.cfd_nodal_flags1 ());
    printf ("CFD flags2 : %d\n", ctl.cfd_nodal_flags2 ());
    printf ("======================================================================\n");

    if (!ctl.mattypes ()) {
	printf ("Materials not present\n");
	printf ("======================================================================\n");
    }

    assert (!ctl.mattypes ());

    return 0;
}

