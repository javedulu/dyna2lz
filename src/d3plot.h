// 
// D3Plot data class
// 
// $Log: d3plot.h,v $
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
#ifndef __D3PLOT_H__
#define __D3PLOT_H__

#include "options.h"

#include <stdio.h>
#include <stdlib.h>

#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>

#include <map>
#include <vector>
#include <set>

#define WORD_SIZE 4


// input source for bunch of d3plots
class D3PlotFile
{
private:
    char* _baseName;
    FILE* _f;
    unsigned int _index;
    std::vector<long> _pos_stack;

public:
    D3PlotFile (const char* fileName);
    ~D3PlotFile ();

    bool readBool ();
    unsigned int readUInt ();
    int readInt ();
    float readFloat ();
    void readBlock (void* buf, unsigned int size);
    void skip (unsigned int size);
    void sayPos ();
    bool openNextFile ();

    void pushPos ();
    void popPos ();
};



class D3PlotControl 
{
private:
    char _model_descr[10*4+1];
    unsigned int _runtime, _rundate, _machine, _codeid, _dimensions;
    float _codever;
    bool _elem_conns;		// element conectivies are present
    bool _mattypes;		// materials types are present
    bool _road_movement;	// data contains rigid road movement
    unsigned int _nodes;
    bool _code_is_new;	
    bool _temperatures;
    bool _cur_geom;
    bool _velocities;
    bool _accelerations;
    unsigned int _num_global_vars; // number of global variables to read each state

    unsigned int _num_8_node_elems;
    unsigned int _num_8_node_mats;
    unsigned int _num_8_node_vals;
    unsigned int _num_8_node_add;

    unsigned int _num_2_node_elems;
    unsigned int _num_2_node_mats;
    unsigned int _num_2_node_vals;

    unsigned int _num_4_node_elems;
    unsigned int _num_4_node_mats;
    unsigned int _num_4_node_vals;
    unsigned int _num_4_node_add;
    unsigned int _num_4_node_int; // number of integration points, MAXINT

    unsigned int _elems_deletion; // MDLOPT

    unsigned int _sph_nodes;
    unsigned int _sph_mats;
    unsigned int _narbs;	// ?
    
    unsigned int _thick_shell_elems;
    unsigned int _thick_shell_mats;
    unsigned int _thick_shell_vals;

    bool _stress_components;
    bool _plastic_strain;
    bool _shell_force_res;
    bool _shell_thickness_energy;
    
    unsigned int _fluid_mats;
    unsigned int _cfd_nodal_flags1;
    unsigned int _cfd_nodal_flags2;

protected:
    bool readBool (FILE* f) const;
    unsigned int readUInt (FILE* f) const;
    int readInt (FILE* f) const;
    float readFloat (FILE* f) const;

public:
    D3PlotControl (D3PlotFile* f);

    unsigned int dimensions () const
        { return _dimensions; };
    const char* model_descr () const
        { return _model_descr; };
    unsigned int runtime () const
        { return _runtime; };
    unsigned int rundate () const
        { return _rundate; };
    unsigned int machine () const
        { return _machine; };
    unsigned int codeid () const
        { return _codeid; };
    float codever () const
        { return _codever; };
    bool elem_conns () const
        { return _elem_conns; };
    bool mattypes () const
        { return _mattypes; }; 
    bool road_movement () const 
        { return _road_movement; };
    unsigned int nodes () const
        { return _nodes; };
    bool code_is_new () const
        { return _code_is_new; };
    bool temperatures () const
        { return _temperatures; };
    bool cur_geom () const
        { return _cur_geom; };
    bool velocities () const
        { return _velocities; };
    bool accelerations () const
        { return _accelerations; };
    unsigned int num_global_vars () const
        { return _num_global_vars; };
    unsigned int num_8_node_elems () const
        { return _num_8_node_elems; };
    unsigned int num_8_node_mats () const
        { return _num_8_node_mats; };
    unsigned int num_8_node_vals () const
        { return _num_8_node_vals; };
    unsigned int num_8_node_add () const
        { return _num_8_node_add; };
    unsigned int num_2_node_elems () const
        { return _num_2_node_elems; };
    unsigned int num_2_node_mats () const
        { return _num_2_node_mats; };
    unsigned int num_2_node_vals () const
        { return _num_2_node_vals; };
    unsigned int num_4_node_elems () const
        { return _num_4_node_elems; };
    unsigned int num_4_node_mats () const
        { return _num_4_node_mats; };
    unsigned int num_4_node_vals () const
        { return _num_4_node_vals; };
    unsigned int num_4_node_add () const
        { return _num_4_node_add; };
    unsigned int num_4_node_int () const
        { return _num_4_node_int; };
    unsigned int elems_deletion () const
        { return _elems_deletion; };
    unsigned int sph_nodes () const
        { return _sph_nodes; };
    unsigned int sph_mats () const
        { return _sph_mats; };
    unsigned int narbs () const
        { return _narbs; };
    unsigned int thick_shell_elems () const
        { return _thick_shell_elems; };
    unsigned int thick_shell_mats () const
        { return _thick_shell_mats; };
    unsigned int thick_shell_vals () const 
        { return _thick_shell_vals; };
    bool stress_components () const
        { return _stress_components; };
    bool plastic_strain () const
        { return _plastic_strain; };
    bool shell_force_res () const
        { return _shell_force_res; };
    bool shell_thickness_energy () const
        { return _shell_thickness_energy; };
    unsigned int fluid_mats () const
        { return _fluid_mats; };
    unsigned int cfd_nodal_flags1 () const
        { return _cfd_nodal_flags1; };
    unsigned int cfd_nodal_flags2 () const 
        { return _cfd_nodal_flags2; };

    unsigned int total_cells () const
        { return _num_8_node_elems + _thick_shell_elems + _num_2_node_elems + _num_4_node_elems; };

    bool istrn () const;
};


typedef struct {
    float x, y, z;
} node_coord_t;


typedef struct {
    float val[3];
} vector_3_t;


typedef struct {
    float val[2];
} vector_2_t;


// abstract generic cell - collection of node IDs
class GenericCell {
private:
    unsigned int _nodes[8];
    unsigned char _pts;
    unsigned int  _partID;
    unsigned int _vtkElemKind;
    
public:
    GenericCell (unsigned int* nodes, unsigned char pts, unsigned int partID, unsigned int vtkElemKind);
    ~GenericCell ();

    unsigned char nodesCount () const
        { return _pts; };

    unsigned int partID () const
        { return _partID; };

    unsigned int node (unsigned char index) const
        { return _nodes[index]; };

    unsigned int vtkElemKind () const
        { return _vtkElemKind; };
};



typedef struct {
    unsigned int nodes[4];
    unsigned int partid;
} elem_4_t;


typedef struct {
    unsigned int nodes[2];
    unsigned int orient, null1, null2, partid;
} elem_2_t;


typedef struct {
    float val[6];
} tensor_t;


/* index of different grids in grids array */
typedef enum {
    gridSolids = 0,
    gridShells = 1,
    gridBeams  = 2,
} grid_kind_t;


typedef enum {
    shellMiddle = 0,
    shellInner  = 1,
    shellOuter  = 2,
} shell_pos_t;


class D3PlotGeometry
{
private:
    D3PlotControl* _ctl;           // just a reference, do not delete
    StateOptions* _opts;
    
    // statistics
    unsigned int _points, _hexas, _lines, _triangles, _quads, _pyramids, _tetras, _wedges;

    // geometry
    node_coord_t* _nodes;
    node_coord_t* _deltas;
    std::vector<GenericCell*> _cells[3];

    // nodes maps
    unsigned int* _local2global[3];
    unsigned int _l2g_size[3];
    
    std::map<unsigned int, unsigned int> _global2local[3];

    // state variables
    bool* _deleted[3];
    std::vector<node_coord_t> _vel;
    std::vector<node_coord_t> _accel;
    std::vector<tensor_t> _sigma[3];
    std::vector<float> _pl_strain[3];
    std::vector<tensor_t> _strain[3];

    // shell-specific fields
    std::vector<tensor_t> _innerSigma;
    std::vector<tensor_t> _outerSigma;
    std::vector<float> _pl_innerStrain;
    std::vector<float> _pl_outerStrain;
    std::vector<tensor_t> _innerStrain;
    std::vector<tensor_t> _outerStrain;

    std::vector<vector_3_t> _bendingMoment;
    std::vector<vector_2_t> _shearResultant;
    std::vector<vector_3_t> _normalResultant;
    std::vector<float> _thickness;
    std::vector<vector_2_t> _elemDepVar;
    std::vector<float> _energy;
    
    bool _stateMode;
    
protected:
    vtkUnstructuredGrid* createGrid (grid_kind_t kind);

    vtkPoints* getPoints (grid_kind_t grid);

    void resolveInvariants (float* res, const tensor_t& data);

    void appendCellArray (vtkUnstructuredGrid* grid, vtkDataArray* array);
    vtkFloatArray* createArray (const char* name, unsigned int components = 1);
    
public:
    D3PlotGeometry (D3PlotFile* f, D3PlotControl* ctl, StateOptions* opts);

    ~D3PlotGeometry ();

    unsigned int getPointsCount () const
        { return _points; };
    unsigned int getHexasCount () const
        { return _hexas; };
    unsigned int getLinesCount () const
        { return _lines; };
    unsigned int getTrianglesCount () const
        { return _triangles; };
    unsigned int getQuadsCount () const
        { return _quads; };
    unsigned int getPyramidsCount () const
        { return _pyramids; };
    unsigned int getTetrasCount () const
        { return _tetras; };
    unsigned int getWedgesCount () const
        { return _wedges; };

    bool save (const char* baseName, int index = -1);

    void updateMaps ();
    void resetState ();
    
    void markDeleted (unsigned int cell);
    void setVelocity (unsigned int node, float* val);
    void setAcceleration (unsigned int node, float* val);

    void setSigma    (int grid, unsigned int localCell, float* val, shell_pos_t shellPos = shellMiddle);
    void setPlStrain (int grid, unsigned int localCell, float val,  shell_pos_t shellPos = shellMiddle);
    void setStrain   (int grid, unsigned int localCell, float* val, shell_pos_t shellPos = shellMiddle);

    void setBendingMoment   (unsigned int localCell, float* val);
    void setShearResultant  (unsigned int localCell, float* val);
    void setNormalResultant (unsigned int localCell, float* val);
    void setThickness       (unsigned int localCell, float  val);
    void setElemDepVal      (unsigned int localCell, float* val);
    void setEnergy          (unsigned int localCell, float  val);
    
    void movePoint (unsigned int id, float* data);
};



// class helps to write multipart results data
class PVDWriter
{
private:
    const char* _baseName;
    std::vector<const char*> _names;
    std::vector<vtkUnstructuredGrid*> _grids;
    bool _pvd_mode;
    int _index;
    
public:
    PVDWriter (const char* baseName, bool pvd_mode, int index);
    ~PVDWriter ();
    
    void appendPart (const char* baseName, vtkUnstructuredGrid* grid);

    void write ();
};



class D3PlotState
{
private:
    StateOptions* _opts;
    D3PlotControl* _ctl;
    D3PlotGeometry* _geo;
    float _time;
    D3PlotFile* _f;

public:
    D3PlotState (StateOptions* opts, D3PlotControl* ctl, D3PlotGeometry* geo, D3PlotFile* f);
    ~D3PlotState ();

    void read ();

    float time () const
        { return _time; };

    void save (const char* baseName, int index);
};


#endif

