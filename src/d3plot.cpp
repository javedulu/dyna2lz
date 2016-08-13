#include "d3plot.h"
#include "options.h"

#include <vector>
#include <map>
#include <set>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/stat.h>
#include <sys/types.h>


// float comparision function
int float_compare (const void* a, const void* b)
{
    float fa = *(float*)a, fb = *(float*)b;
    return fa < fb ? -1 : 1;
}


int float_compare_rev (const void* a, const void* b)
{
    return -float_compare (a, b);
}


// --------------------------------------------------
// D3PlotFile
// --------------------------------------------------
D3PlotFile::D3PlotFile (const char* fileName)
{
    _baseName = strdup (fileName);
    _f = 0;
    _index = 0;
}


D3PlotFile::~D3PlotFile ()
{
    if (_baseName)
        free (_baseName);
    if (_f)
        fclose (_f);
}


void D3PlotFile::readBlock (void* buf, unsigned int size)
{
    if (!_f)
        openNextFile ();

    if (!fread (buf, size, 1, _f)) {
        openNextFile ();
        int res = fread (buf, size, 1, _f);
        if (!res)
            // here we must throw an exception
            ;
    }
}


bool D3PlotFile::openNextFile ()
{
    if (_f)
        fclose (_f);

    static char buf[1024];

    if (_index)
        sprintf (buf, "%s%02d", _baseName, _index);
    else
        sprintf (buf, "%s", _baseName);
    _f = fopen (buf, "rb");

    _index++;

    return _f;
}



bool D3PlotFile::readBool ()
{
    return readUInt () != 0;
}


unsigned int D3PlotFile::readUInt ()
{
    unsigned int res;
    readBlock (&res, sizeof (unsigned int));
    return res;
}


int D3PlotFile::readInt ()
{
    int res;
    readBlock (&res, sizeof (int));
    return res;
}


float D3PlotFile::readFloat ()
{
    float res;
    readBlock (&res, sizeof (float));
    return res;
}


void D3PlotFile::skip (unsigned int size)
{
    while (size)
    {
        readInt ();
        size -= sizeof (int);
    }
}


void D3PlotFile::sayPos ()
{
    printf ("Position: %x\n", ftell (_f));
}


void D3PlotFile::pushPos ()
{
    _pos_stack.push_back (ftell (_f));
}


void D3PlotFile::popPos ()
{
    long pos = _pos_stack.back ();
    _pos_stack.pop_back ();
    fseek (_f, pos, SEEK_SET);
}



// --------------------------------------------------
// D3PlotControl class
// --------------------------------------------------
D3PlotControl::D3PlotControl (D3PlotFile* f)
{
    if (!f)
        throw;

    // fetch model description
    memset (_model_descr, 0, sizeof (_model_descr));
    f->readBlock (_model_descr, WORD_SIZE*10);

    _runtime = f->readUInt ();
    _rundate = f->readUInt ();
    _machine = f->readUInt ();
    _codeid  = f->readUInt ();
    _codever = f->readFloat ();

    // decode amount of dimensions
    unsigned int tmp_word = f->readUInt ();

    _road_movement = _elem_conns = _mattypes = false;

    switch (tmp_word) {
    case 2:
    case 3:
        _dimensions = tmp_word;
        break;
    case 5:
    case 7:
        _mattypes = _elem_conns = true;
        _dimensions = 3;
        break;
    case 4:
        _elem_conns = true;
        _dimensions = 3;
        break;
    default:
        _dimensions = 3;
        _road_movement = true;
    }

    _nodes = f->readUInt ();

    // is data generated by the new code?
    _code_is_new = f->readUInt () == 6;

    // amount of global variables to read in each state
    _num_global_vars = f->readUInt ();

    // a lot of flags
    _temperatures = f->readBool ();
    _cur_geom = f->readBool ();
    _velocities = f->readBool ();
    _accelerations = f->readBool ();

    // 8 node solid elements
    _num_8_node_elems = f->readUInt ();
    _num_8_node_mats  = f->readUInt ();
    f->skip (2*sizeof (int));
    _num_8_node_vals = f->readUInt ();

    // 2 node 1D elements
    _num_2_node_elems = f->readUInt ();
    _num_2_node_mats  = f->readUInt ();
    _num_2_node_vals  = f->readUInt ();

    // 4 node 2D elements
    _num_4_node_elems = f->readUInt ();
    _num_4_node_mats  = f->readUInt ();
    _num_4_node_vals  = f->readUInt ();

    _num_8_node_add   = f->readUInt (); // amount of additional vals per solid elem
    _num_4_node_add   = f->readUInt (); // amount of additional vals per shell elem

    int int_tmp = f->readInt ();

    if (int_tmp >= 0) {
        _num_4_node_int = int_tmp;
        _elems_deletion = 0;
    } else
        if (int_tmp < -10000) {
            _elems_deletion = 2;
            _num_4_node_int = -int_tmp - 10000;
        } else
            if (int_tmp < 0) {
                _elems_deletion = 1;
                _num_4_node_int = -int_tmp;
            }

    _sph_nodes = f->readUInt ();
    _sph_mats  = f->readUInt ();
    _narbs = f->readUInt ();

    _thick_shell_elems = f->readUInt ();
    _thick_shell_mats  = f->readUInt ();
    _thick_shell_vals  = f->readUInt ();

    _stress_components = f->readInt () == 1000;
    _plastic_strain = f->readUInt () == 1000;
    _shell_force_res = f->readUInt () == 1000;
    _shell_thickness_energy = f->readUInt () == 1000;

    _fluid_mats = f->readUInt ();
    _cfd_nodal_flags1 = f->readUInt ();
    _cfd_nodal_flags2 = f->readUInt ();

    // skip blank fields
    f->skip (14 * WORD_SIZE);
}


bool D3PlotControl::readBool (FILE* f) const
{
    unsigned int tmp;
    fread (&tmp, WORD_SIZE, 1, f);
    return tmp == 1;
}


unsigned int D3PlotControl::readUInt (FILE* f) const
{
    unsigned int tmp;
    fread (&tmp, WORD_SIZE, 1, f);
    return tmp;
}


int D3PlotControl::readInt (FILE* f) const
{
    int tmp;
    fread (&tmp, WORD_SIZE, 1, f);
    return tmp;
}


float D3PlotControl::readFloat (FILE* f) const
{
    float tmp;
    fread (&tmp, WORD_SIZE, 1, f);
    return tmp;
}


bool D3PlotControl::istrn () const
{
    int a = _num_4_node_vals;

    if (_stress_components)
        a -= _num_4_node_int*6;

    if (_plastic_strain)
        a -= _num_4_node_int;

    a -= _num_4_node_add * _num_4_node_int;

    if (_shell_force_res)
        a -= 8;

    if (_shell_thickness_energy)
        a -= 4;

    if (a > 1)
        return true;

//     int b = _num_8_node_elems + _thick_shell_elems -
//             INT_MAX*(6*1000*_stress_components + 1000*_plastic_strain + _num_8_node_add);
// //    printf ("B = %d\n", b);
//     if (_num_8_node_elems+_thick_shell_elems > 0 && b > 1)
//         return true;

    return false;
}



// --------------------------------------------------
// GenericCell
// --------------------------------------------------
GenericCell::GenericCell (unsigned int* nodes, unsigned char pts, unsigned int partID, unsigned int elemKind)
    : _pts (pts),
      _partID (partID),
      _elemKind (elemKind)
{
//    _nodes = (unsigned int*)malloc (sizeof (unsigned int) * pts);
    memcpy (_nodes, nodes, sizeof (unsigned int) * pts);
}



GenericCell::~GenericCell ()
{
//    free (_nodes);
}




// --------------------------------------------------
// D3PlotGeometry class
// --------------------------------------------------
// File handle must be positioned at the begining
// of gemetry block
D3PlotGeometry::D3PlotGeometry (D3PlotFile* f, D3PlotControl* ctl, StateOptions* opts)
    : _ctl  (ctl),
      _opts (opts),
      _stateMode (false),
      _nodes (0)
{
    int i, j;

    _points = _hexas = _lines = _triangles = _quads = _pyramids = _tetras = _wedges = 0;

    for (i = 0; i < 3; i++) {
        _deleted[i] = 0;
        _local2global[i] = 0;
    }
    
    // --[ Nodes ]------------------------------------------------
    // read points
    f->sayPos ();
    _nodes  = (node_coord_t*)malloc (ctl->nodes () * sizeof (node_coord_t));
    _deltas = (node_coord_t*)malloc (ctl->nodes () * sizeof (node_coord_t));

    memset (_deltas, 0, ctl->nodes () * sizeof (node_coord_t));
    f->readBlock (_nodes, sizeof (node_coord_t) * ctl->nodes ());
    _points = ctl->nodes ();

    int id = 0;

    // --[ Solids ]------------------------------------------------
    for (i = 0; i < ctl->num_8_node_elems () + ctl->thick_shell_elems (); i++) {
        unsigned int nodes[8], partID, elemKind;
        int pts = 0;

        f->readBlock (nodes, sizeof (nodes));
        partID = f->readUInt ();

        for (j = 0; j < 8; j++) {
            nodes[j]--;
            if (j == 0 || nodes[j] != nodes[j-1])
                pts++;
            else
                break;
        }

        switch (pts) {
        case 8:
            elemKind = VTK_HEXAHEDRON;
            _hexas++;
            break;

        case 5:
            elemKind = VTK_PYRAMID;
            _pyramids++;
            break;

        case 4:
            elemKind = VTK_TETRA;
            _tetras++;
            break;

        case 6:
            elemKind = VTK_WEDGE;
            _wedges++;
            break;

        default:
            printf ("Unknown amount of vertices = %d\n", pts);
        }

        _cells[gridSolids].push_back (new GenericCell (nodes, pts, partID, elemKind));
    }

    // --[ Beams ]------------------------------------------------
    for (i = 0; i < ctl->num_2_node_elems (); i++) {
        unsigned int tmp[5];
        unsigned int partID;
        f->readBlock (tmp, sizeof (tmp));
        partID = f->readUInt ();

        tmp[0]--;
        tmp[1]--;

        _cells[gridBeams].push_back (new GenericCell (tmp, 2, partID, VTK_LINE));
        _lines++;
    }

    // --[ Shells ]------------------------------------------------
    for (i = 0; i < ctl->num_4_node_elems (); i++) {
        unsigned int points[4];
        unsigned int partID;

        f->readBlock (points, sizeof (points));
        partID = f->readUInt ();

        for (int j = 0; j < 4; j++)
            points[j]--;

        if (points[3] == points[2]) {
            _cells[gridShells].push_back (new GenericCell (points, 3, partID, VTK_TRIANGLE));
            _triangles++;
        }
        else {
            _cells[gridShells].push_back (new GenericCell (points, 4, partID, VTK_QUAD));
            _quads++;
        }
    }

    updateMaps ();
}


D3PlotGeometry::~D3PlotGeometry ()
{
    std::vector<GenericCell*>::iterator it;

    for (int i = 0; i < 3; i++) {

        it = _cells[i].begin ();

        while (it != _cells[i].end ())
            delete *(it++);
        _cells[i].clear ();

        if (_local2global[i])
            free (_local2global[i]);
        
        _global2local[i].clear ();
        free (_deleted[i]);
        _sigma[i].clear ();
        _pl_strain[i].clear ();
        _strain[i].clear ();
    }

    free (_nodes);
    free (_deltas);
    _vel.clear ();
    _accel.clear ();
    _innerSigma.clear ();
    _outerSigma.clear ();
    _pl_innerStrain.clear ();
    _pl_outerStrain.clear ();
    _innerStrain.clear ();
    _outerStrain.clear ();
}


void D3PlotGeometry::resolveInvariants (float* res, const tensor_t& data)
{
    float sigma[6] = {
        data.val[0], data.val[1], data.val[2],
        data.val[3], data.val[4], data.val[5],
    };
    float alpha = 0.0, koefA = -(sigma[0]+sigma[1]+sigma[2]), koefB, koefC, kP, kQ, fi;

    koefB = sigma[0]*sigma[1] + sigma[1]*sigma[2] + sigma[2]*sigma[0] -
            sigma[3]*sigma[3] - sigma[4]*sigma[4] - sigma[5]*sigma[5];

    koefC = sigma[0]*sigma[1]*sigma[2] - 2*sigma[3]*sigma[4]*sigma[5] +
            sigma[0]*sigma[4]*sigma[4] + sigma[1]*sigma[5]*sigma[5] + sigma[2]*sigma[3]*sigma[3];

    kP = -(koefA * koefA / 3.0) + koefB;

    kQ = 2*koefA*koefA*koefA - koefA*koefB/3.0 + koefC;
    fi = -kQ / 2.0 / sqrt (-kP*kP*kP/27.0);

    if (fabs (fi) >= 1)
        alpha = 0.0;
    else
        alpha = acos (fi) / 3.0;

    res[0] = 2*sqrt (-kP/3.0) * cos (alpha) - koefA/3.0;
    res[1] = -2*sqrt (-kP/3.0) * cos (alpha + 1.047197551197) - koefA/3.0;
    res[2] = -2*sqrt (-kP/3.0) * cos (alpha - 1.047197551197) - koefA/3.0;

    qsort (res, 3, sizeof (float), float_compare_rev);
}


void D3PlotGeometry::appendCellArray (vtkUnstructuredGrid* grid, vtkDataArray* array)
{
    if (array) {
        grid->GetCellData ()->AddArray (array);
        array->Delete ();
    }
}


vtkFloatArray* D3PlotGeometry::createArray (const char* name, unsigned int components)
{
    vtkFloatArray* res = vtkFloatArray::New ();

    res->SetName (name);
    res->SetNumberOfComponents (components);

    return res;
}



vtkUnstructuredGrid* D3PlotGeometry::createGrid (grid_kind_t kind)
{
    vtkUnstructuredGrid* grid = vtkUnstructuredGrid::New ();
    vtkPoints* points = getPoints (kind);

    grid->SetPoints (points);
    points->Delete ();

    // cells
    std::vector<GenericCell*>::iterator it = _cells[kind].begin ();
    vtkUnsignedIntArray* partIDField      = vtkUnsignedIntArray::New ();
    vtkUnsignedIntArray* elementTypeField = vtkUnsignedIntArray::New ();

    partIDField->SetName ("PartID");
    elementTypeField->SetName ("ElementType");

    vtkFloatArray* sigmaField = 0;
    vtkFloatArray* vm_stressField = 0;
    vtkFloatArray* pri_stressField = 0;
    vtkFloatArray* hydroPressureField = 0;
    vtkFloatArray* pri_shearStressField = 0;
    vtkFloatArray* oct_shearStressField = 0;
    
    if (_stateMode && _sigma[kind].size ()) {
        sigmaField           = createArray ("Sigma", 6);
        vm_stressField       = createArray ("Von Mizes Stress");
        pri_stressField      = createArray ("Principal Stress", 3);
        hydroPressureField   = createArray ("Hydrostatic Pressure");
        pri_shearStressField = createArray ("Principal Shear Stress", 3); 
        oct_shearStressField = createArray ("Octahedral Shear Stress");
    }

    vtkFloatArray* plStrainField = 0;

    if (_stateMode && _pl_strain[kind].size ())
        plStrainField = createArray ("Plastic Strain");

    vtkFloatArray* strainField = 0;
    vtkFloatArray* pri_strainField = 0;

    if (_stateMode && _ctl->istrn () && _strain[kind].size ()) {
        strainField = createArray ("Strain", 6); 
        pri_strainField = createArray ("Principal Strain", 3);
    }

    vtkUnsignedCharArray* deletedField = 0;

    if (_opts->keepDeleted () && _deleted[kind]) {
        deletedField = vtkUnsignedCharArray::New ();
        deletedField->SetName ("Deleted");
    }

    // shell-specific fields
    vtkFloatArray* innerSigmaField = 0;
    vtkFloatArray* outerSigmaField = 0;
    vtkFloatArray* innerPlStrainField = 0;
    vtkFloatArray* outerPlStrainField = 0;
    vtkFloatArray* innerStrainField = 0;
    vtkFloatArray* outerStrainField = 0;

    vtkFloatArray* bendingMomentField = 0;
    vtkFloatArray* shearResultantField = 0;
    vtkFloatArray* normalResultantField = 0;
    vtkFloatArray* thicknessField = 0;
    vtkFloatArray* elemDepValField = 0;
    vtkFloatArray* energyField = 0;
    
    if (_stateMode && kind == gridShells) {
        innerSigmaField = createArray ("InnerSigma", 6);
        outerSigmaField = createArray ("OuterSigma", 6);
        innerPlStrainField = createArray ("InnerPlasticStrain");
        outerPlStrainField = createArray ("OuterPlasticStrain");

        bendingMomentField = createArray ("Bending Moment", 3);
        shearResultantField = createArray ("Shear Resultant", 2);
        normalResultantField = createArray ("Normal Resultant", 3);
        thicknessField = createArray ("Thickness");
        elemDepValField = createArray ("Element Depended Value", 2);
        energyField = createArray ("Internal Energy");
        
        if (_ctl->istrn ()) {
            innerStrainField = createArray ("InnerStrain", 6);
            outerStrainField = createArray ("OuterStrain", 6);
        }
    }

    int index = 0;
    bool notCheckDel = _opts->keepDeleted () || !_deleted[kind];
    int i;

    while (it != _cells[kind].end ()) {
        vtkIdType pts[8];

        if (_opts->partIDCheck ((*it)->partID ()) && (notCheckDel || !_deleted[kind][index])) {
            for (i = 0; i < (*it)->nodesCount (); i++)
                pts[i] = _global2local[kind][(*it)->node (i)];

            grid->InsertNextCell ((*it)->elemKind (), (*it)->nodesCount (), pts);
            partIDField->InsertNextValue ((*it)->partID ());
            elementTypeField->InsertNextValue ((*it)->elemKind ());

            if (deletedField)
                deletedField->InsertNextValue (_deleted[kind][index]);
            
            if (sigmaField) {
                sigmaField->InsertNextTuple (_sigma[kind][index].val);

                // calculate custom fields
                float a, b, c, d, e, f;
                float sigma[6] = {
                    _sigma[kind][index].val[0], _sigma[kind][index].val[1], _sigma[kind][index].val[2],
                    _sigma[kind][index].val[3], _sigma[kind][index].val[4], _sigma[kind][index].val[5],
                };
                
                a = sigma[0] - sigma[1];
                b = sigma[1] - sigma[2];
                c = sigma[2] - sigma[0];

                d = sigma[3];
                e = sigma[4];
                f = sigma[5];

                vm_stressField->InsertNextValue (sqrt ((a*a + b*b + c*c + 6*(d*d+e*e+f*f)) / 2));

                a = -(sigma[0]+sigma[1]+sigma[2]);

                hydroPressureField->InsertNextValue (a / 3.0);

                float data[3];
                
                resolveInvariants (data, _sigma[kind][index]);
                pri_stressField->InsertNextTuple (data);

                float data2[3] = {
                    (data[0]-data[1]),
                    (data[1]-data[2]),
                    (data[2]-data[0])
                };

                float oct_shear = sqrt (data2[0]*data2[0] + data2[1]*data2[1] + data2[2]*data2[2]) / 3;

                oct_shearStressField->InsertNextValue (oct_shear);
                
                data2[0] /= 2;
                data2[1] /= 2;
                data2[2] /= 2;
                
                pri_shearStressField->InsertNextTuple (data2);
                
            }
            if (plStrainField)
                plStrainField->InsertNextValue (_pl_strain[kind][index]);

            if (strainField) {
                strainField->InsertNextTuple (_strain[kind][index].val);

                float data[3];
                
                resolveInvariants (data, _strain[kind][index]);
                pri_strainField->InsertNextTuple (data);
            }

            if (innerSigmaField) {
                innerSigmaField->InsertNextTuple (_innerSigma[index].val);
                outerSigmaField->InsertNextTuple (_outerSigma[index].val);
                innerPlStrainField->InsertNextValue (_pl_innerStrain[index]);
                outerPlStrainField->InsertNextValue (_pl_outerStrain[index]);

                if (_ctl->istrn ()) {
                    innerStrainField->InsertNextTuple (_innerStrain[index].val);
                    outerStrainField->InsertNextTuple (_outerStrain[index].val);
                }
                
                bendingMomentField->InsertNextTuple (_bendingMoment[index].val);
                shearResultantField->InsertNextTuple (_shearResultant[index].val);
                normalResultantField->InsertNextTuple (_normalResultant[index].val);
                thicknessField->InsertNextValue (_thickness[index]);
                elemDepValField->InsertNextTuple (_elemDepVar[index].val);
                energyField->InsertNextValue (_energy[index]);
            }
        }

        index++;
        it++;
    }

    appendCellArray (grid, partIDField);
    appendCellArray (grid, elementTypeField);
    appendCellArray (grid, deletedField);
    appendCellArray (grid, sigmaField);
    appendCellArray (grid, vm_stressField);
    appendCellArray (grid, pri_stressField);
    appendCellArray (grid, hydroPressureField);
    appendCellArray (grid, pri_shearStressField);
    appendCellArray (grid, oct_shearStressField);
    appendCellArray (grid, plStrainField);
    appendCellArray (grid, strainField);
    appendCellArray (grid, pri_strainField);

    appendCellArray (grid, innerSigmaField);
    appendCellArray (grid, outerSigmaField);
    appendCellArray (grid, outerPlStrainField);
    appendCellArray (grid, innerStrainField);
    appendCellArray (grid, outerStrainField);

    appendCellArray (grid, bendingMomentField);
    appendCellArray (grid, shearResultantField);
    appendCellArray (grid, normalResultantField);
    appendCellArray (grid, thicknessField);
    appendCellArray (grid, elemDepValField);
    appendCellArray (grid, energyField);
    
    if (_stateMode) {
        vtkFloatArray* velField = 0;
        vtkFloatArray* accField = 0;
        vtkFloatArray* deltaField = 0;
        vtkFloatArray* coordsField = 0;
        
        if (_ctl->velocities ())
            velField = createArray ("Velocity", 3);

        if (_ctl->accelerations ())
            accField = createArray ("Acceleration", 3);

        deltaField  = createArray ("Delta Movements", 3);
        coordsField = createArray ("Coords", 3);

        // nodal values
        for (int i = 0; i < _l2g_size[kind]; i++) {
            unsigned int id = _local2global[kind][i];
            if (velField)
                velField->InsertNextTuple ((float*)&_vel[id]);
            if (accField)
                accField->InsertNextTuple ((float*)&_accel[id]);

            deltaField->InsertNextTuple ((float*)&_deltas[id]);
            coordsField->InsertNextTuple ((float*)&_nodes[id]);
        }

        if (velField) {
            grid->GetPointData ()->AddArray (velField);
            velField->Delete ();
        }
        if (accField) {
            grid->GetPointData ()->AddArray (accField);
            accField->Delete ();
        }

        grid->GetPointData ()->AddArray (deltaField);
        deltaField->Delete ();

        grid->GetPointData ()->AddArray (coordsField);
        coordsField->Delete ();
    }

    return grid;
}


// update maps local_pt->global_pt && global->local
void D3PlotGeometry::updateMaps ()
{
    for (int grid = 0; grid < 3; grid++) {
        if (!_cells[grid].size ())
            continue;

        if (_local2global[grid])
            free (_local2global[grid]);
        _local2global[grid] = (unsigned int*)malloc (sizeof (unsigned int) * _points);
        _l2g_size[grid] = 0;
        _global2local[grid].clear ();

        std::vector<GenericCell*>::const_iterator solid = _cells[grid].begin ();
        int index = 0;

        while (solid != _cells[grid].end ()) {
            if (_opts->partIDCheck ((*solid)->partID ()))
                if (!_stateMode || _opts->keepDeleted () || !_deleted[grid][index] ) {
                    for (int i = 0; i < (*solid)->nodesCount (); i++) {
                        // we have global node id
                        int g;

                        g = (*solid)->node (i);

                        std::map<unsigned int, unsigned int>::const_iterator local = _global2local[grid].find (g);

                        if (local == _global2local[grid].end ()) {
                            _global2local[grid][g] = _l2g_size[grid];
                            _local2global[grid][_l2g_size[grid]++] = g;
                        }
                    }
                }

            solid++;
            index++;
        }
    }
}



vtkPoints* D3PlotGeometry::getPoints (grid_kind_t grid)
{
    vtkPoints* points = vtkPoints::New ();

    for (int i = 0; i < _l2g_size[grid]; i++)
        points->InsertNextPoint ((float*)&_nodes[_local2global[grid][i]]);

    return points;
}



bool D3PlotGeometry::save (const char* baseName, int index)
{
    PVDWriter writer (baseName, _opts->pvdMode (), index);
    const char* names[] = { "solids", "shells", "beams" };

    for (int i = 0; i < 3; i++)
        if (_cells[i].size ())
            writer.appendPart (names[i], createGrid ((grid_kind_t)i));

    writer.write ();

    return true;
}


void D3PlotGeometry::resetState ()
{
    int i;

    for (i = 0; i < 3; i++) {
        if (!_deleted[i] && _cells[i].size () > 0)
            _deleted[i] = (bool*)malloc (_cells[i].size () * sizeof (bool));
    
        memset (_deleted[i], 0, _cells[i].size () * sizeof (bool));

        if (i != gridBeams) {
            _sigma[i].resize (_cells[i].size ());
            _pl_strain[i].resize (_cells[i].size ());
            if (_ctl->istrn () && i != gridShells)
                _strain[i].resize (_cells[i].size ());
        }
    }

    _innerSigma.resize (_cells[gridShells].size ());
    _outerSigma.resize (_cells[gridShells].size ());
    _pl_innerStrain.resize (_cells[gridShells].size ());
    _pl_outerStrain.resize (_cells[gridShells].size ());

    _bendingMoment.resize (_cells[gridShells].size ());
    _shearResultant.resize (_cells[gridShells].size ());
    _normalResultant.resize (_cells[gridShells].size ());
    _thickness.resize (_cells[gridShells].size ());
    _elemDepVar.resize (_cells[gridShells].size ());
    _energy.resize (_cells[gridShells].size ());
    
    if (_ctl->istrn ()) {
        _innerStrain.resize (_cells[gridShells].size ());
        _outerStrain.resize (_cells[gridShells].size ());
    }

    if (_ctl->velocities ())
        _vel.resize (_ctl->nodes ());

    if (_ctl->accelerations ())
        _accel.resize (_ctl->nodes ());

    
    _stateMode = true;
}


void D3PlotGeometry::markDeleted (unsigned int cell)
{
    for (int i = 0; i < 3; i++)
        if (cell > _cells[i].size ())
            cell -= _cells[i].size ();
        else {
            _deleted[i][cell] = true;
            break;
        }
}


void D3PlotGeometry::movePoint (unsigned int id, float* data)
{
    _deltas[id].x = data[0]-_nodes[id].x;
    _deltas[id].y = data[1]-_nodes[id].y;
    _deltas[id].z = data[2]-_nodes[id].z;

    _nodes[id].x = data[0];
    _nodes[id].y = data[1];
    _nodes[id].z = data[2];
}


void D3PlotGeometry::setVelocity (unsigned int node, float* val)
{
    _vel[node].x = val[0];
    _vel[node].y = val[1];
    _vel[node].z = val[2];
}


void D3PlotGeometry::setAcceleration (unsigned int node, float* val)
{
    _accel[node].x = val[0];
    _accel[node].y = val[1];
    _accel[node].z = val[2];
}


void D3PlotGeometry::setSigma (int grid, unsigned int cell, float* val, shell_pos_t shellPos)
{
    if (shellPos != shellMiddle && grid != gridShells)
        return;

    switch (shellPos) {
    case shellMiddle:
        for (int j = 0; j < 6; j++)
            _sigma[grid][cell].val[j] = val[j];
        break;
    case shellInner:
        for (int j = 0; j < 6; j++)
            _innerSigma[cell].val[j] = val[j];
        break;
    case shellOuter:
        for (int j = 0; j < 6; j++)
            _outerSigma[cell].val[j] = val[j];
        break;
    }
}


void D3PlotGeometry::setPlStrain (int grid, unsigned int cell, float val, shell_pos_t shellPos)
{
    if (shellPos != shellMiddle && grid != gridShells)
        return;

    switch (shellPos) {
    case shellMiddle:
        _pl_strain[grid][cell] = val;
        break;
    case shellInner:
        _pl_innerStrain[cell] = val;
        break;
    case shellOuter:
        _pl_outerStrain[cell] = val;
        break;
    }
}


void D3PlotGeometry::setStrain   (int grid, unsigned int cell, float* val, shell_pos_t shellPos)
{
    if (shellPos != shellMiddle && grid != gridShells)
        return;

    switch (shellPos) {
    case shellMiddle:
        for (int j = 0; j < 6; j++)
            _strain[grid][cell].val[j] = val[j];
        break;
    case shellInner:
        for (int j = 0; j < 6; j++)
            _innerStrain[cell].val[j] = val[j];
        break;
    case shellOuter:
        for (int j = 0; j < 6; j++)
            _outerStrain[cell].val[j] = val[j];
        break;
    }
}



void D3PlotGeometry::setBendingMoment (unsigned int localCell, float* val)
{
    for (int i = 0; i < 3; i++)
        _bendingMoment[localCell].val[i] = val[i];
}


void D3PlotGeometry::setShearResultant (unsigned int localCell, float* val)
{
    for (int i = 0; i < 2; i++)
        _shearResultant[localCell].val[i] = val[i];
}


void D3PlotGeometry::setNormalResultant (unsigned int localCell, float* val)
{
    for (int i = 0; i < 3; i++)
        _normalResultant[localCell].val[i] = val[i];
}


void D3PlotGeometry::setThickness (unsigned int localCell, float  val)
{
    _thickness[localCell] = val;
}


void D3PlotGeometry::setElemDepVal (unsigned int localCell, float* val)
{
    for (int i = 0; i < 2; i++)
        _elemDepVar[localCell].val[i] = val[i];
}


void D3PlotGeometry::setEnergy (unsigned int localCell, float  val)
{
    _energy[localCell] = val;
}



// --------------------------------------------------
// pvdwriter
// --------------------------------------------------
PVDWriter::PVDWriter (const char* baseName, bool pvd_mode, int index)
    : _baseName (baseName),
      _pvd_mode (pvd_mode),
      _index (index)
{
}


PVDWriter::~PVDWriter ()
{
    std::vector<vtkUnstructuredGrid*>::iterator it = _grids.begin ();

    while (it != _grids.end ()) {
        (*it)->Delete ();
        it++;
    }
}



void PVDWriter::appendPart (const char* baseName, vtkUnstructuredGrid* grid)
{
    _names.push_back (baseName);
    _grids.push_back (grid);
}


void PVDWriter::write ()
{
    static char buf[1024];

    if (_pvd_mode) {
        // create parts directory
        mkdir (_baseName, 0777);

        // create pvd file
        sprintf (buf, "%s.pvd", _baseName);
        FILE* f = fopen (buf, "w+");

        if (!f)
            // throw an exception (TODO)
            return;

        // header
        fprintf (f, "<?xml version=\"1.0\"?>\n");
        fprintf (f, "<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">\n");
        fprintf (f, "<Collection>\n");

        // body
        int index = 0;
        std::vector<const char*>::const_iterator it = _names.begin ();

        const char* p = strrchr (_baseName, '/');

        if (!p)
            p = _baseName;
        else
            p++;

        while (it != _names.end ()) {
            fprintf (f, "<DataSet part=\"%d\" file=\"%s/%s.vtu\"/>\n", index, p, *it);
            it++;
            index++;
        }

        // footer
        fprintf (f, "</Collection>\n");
        fprintf (f, "</VTKFile>\n");

        fclose (f);

        // write datasets
        std::vector<vtkUnstructuredGrid*>::const_iterator _it = _grids.begin ();
        it = _names.begin ();

        while (_it != _grids.end ()) {
            vtkXMLUnstructuredGridWriter* writer = vtkXMLUnstructuredGridWriter::New ();

            if (_index >= 0)
                sprintf (buf, "%s/%s_%05d.vtu", _baseName, *it, _index);
            else
                sprintf (buf, "%s/%s.vtu", _baseName, *it);

            writer->SetInput (*_it);
            writer->SetFileName (buf);
            writer->Write ();
            writer->Delete ();
            _it++;
            it++;
        }
    } else {                    // if not pvd_mode
        std::vector<vtkUnstructuredGrid*>::const_iterator _it = _grids.begin ();
        std::vector<const char*>::const_iterator it = _names.begin ();

        while (_it != _grids.end ()) {
            vtkXMLUnstructuredGridWriter* writer = vtkXMLUnstructuredGridWriter::New ();

            if (_index >= 0)
                sprintf (buf, "%s_%s_%05d.vtu", _baseName, *it, _index);
            else
                sprintf (buf, "%s_%s.vtu", _baseName, *it);

            writer->SetInput (*_it);
            writer->SetFileName (buf);
            writer->Write ();
            writer->Delete ();
            _it++;
            it++;
        }
    }

}


// --------------------------------------------------
// D3PlotState
// --------------------------------------------------
D3PlotState::D3PlotState (StateOptions* opts, D3PlotControl* ctl, D3PlotGeometry* geo, D3PlotFile* f)
    : _opts (opts),
      _ctl (ctl),
      _geo (geo),
      _f (f)
{
    _time = f->readFloat ();

    if (_time < 0) {
        if (!f->openNextFile ())
            throw 0;            // End Of State
        _time = f->readFloat ();
    }
}


D3PlotState::~D3PlotState ()
{
}


void D3PlotState::read ()
{
    int i;

    _f->skip (_ctl->num_global_vars () * 4);

    // here we must fetch deleted nodes/cells set to prevent their
    // insertion into the fields
    unsigned int skipWords =
        _ctl->nodes () * 3 +
        _ctl->velocities () * _ctl->nodes () * 3 +
        _ctl->accelerations () * _ctl->nodes () * 3 +
        (_ctl->num_8_node_elems () + _ctl->thick_shell_elems ()) * (7 + _ctl->num_8_node_add ()) +
        _ctl->num_2_node_elems () * (7+6) +
        _ctl->num_4_node_elems () * _ctl->num_4_node_vals ();

    _f->pushPos ();
    _f->skip (skipWords * 4);
    for (i = 0; i < _ctl->total_cells (); i++)
        if (_f->readFloat () < 0.5)
            _geo->markDeleted (i);
    _f->popPos ();

    // read new nodes coordinates
    for (i = 0; i < _ctl->nodes (); i++) {
        float x[3];
        _f->readBlock (x, sizeof (x));
        _geo->movePoint (i, x);
    }

    if (_ctl->velocities ()) {
        for (i = 0; i < _ctl->nodes (); i++) {
            float v[3];
            _f->readBlock (v, sizeof (v));
            _geo->setVelocity (i, v);
        }
    }

    if (_ctl->accelerations ()) {
        for (i = 0; i < _ctl->nodes (); i++) {
            float a[3];
            _f->readBlock (a, sizeof (a));
            _geo->setAcceleration (i, a);
        }
    }

    _geo->updateMaps ();

    bool istrn = _ctl->istrn ();
    int rest;

    rest = _ctl->num_8_node_add ();
    if (_ctl->num_8_node_add () >= 6)
        rest -= 6;
    
    for (i = 0; i < _ctl->num_8_node_elems () + _ctl->thick_shell_elems (); i++) {
        float v[6], strain;

        _f->readBlock (v, sizeof (v));
        strain = _f->readFloat ();

        _geo->setSigma (gridSolids, i, v);
        _geo->setPlStrain (gridSolids, i, strain);

        if (istrn && _ctl->num_8_node_add () >= 6) {
            _f->readBlock (v, sizeof (v));
            _geo->setStrain (gridSolids, i, v);
        }

        // additional values skipped (TODO)
        _f->skip (rest * sizeof (float));
    }

    for (i = 0; i < _ctl->num_2_node_elems (); i++) {
//         float v[6], strain;

//         _f->readBlock (v, sizeof (v));
//         strain = _f->readFloat ();

        // beam elements data are skipped completely (TODO)
        _f->skip (6*sizeof (float));

    }

    for (i = 0; i < _ctl->num_4_node_elems (); i++) {
        float v[6], strain;
        rest = _ctl->num_4_node_vals ();

        for (int j = 0; j < 3; j++) {
            _f->readBlock (v, sizeof (v));
            strain = _f->readFloat ();

            _geo->setSigma (gridShells, i, v, (shell_pos_t)j);
            _geo->setPlStrain (gridShells, i, strain, (shell_pos_t)j);

            _f->skip (_ctl->num_4_node_add ()*4);
            rest -= 7;
        }

        float v3[3], v2[2];
        
        _f->readBlock (v3, sizeof (v3));
        _geo->setBendingMoment (i, v3);
        _f->readBlock (v2, sizeof (v2));
        _geo->setShearResultant (i, v2);
        _f->readBlock (v3, sizeof (v3));
        _geo->setNormalResultant (i, v3);

        _geo->setThickness (i, _f->readFloat ());

        _f->readBlock (v2, sizeof (v2));
        _geo->setElemDepVal (i, v2);
        
        rest -= 11;
        
        if (istrn) {
            _f->readBlock (v, sizeof (v));
            _geo->setStrain (gridShells, i, v, shellInner);
            _f->readBlock (v, sizeof (v));
            _geo->setStrain (gridShells, i, v, shellOuter);
            rest -= 12;
        }

        _geo->setEnergy (i, _f->readFloat ());
        rest--;

        if (rest > 0)
            _f->skip (rest*4);
    }

    _f->skip (_ctl->total_cells () * 4);
}



void D3PlotState::save (const char* baseName, int index)
{
    _geo->save (baseName, index);
}
