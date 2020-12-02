# This processes all Timestep directories in the current LaMEM 
# directory and creates visualizations of them all
using Printf

# Read LaMEM routines
include("../../scripts/julia/ReadLaMEM_Timestep.jl")

# define a macro that finds all directories with a certain pattern
searchdir(path,key) = filter(x->occursin(key,x), readdir(path))

if length(ARGS)>0
    FileName    =   ARGS[1];            # command line argument
    FileName    = "$FileName.pvtu"
else
    FileName    =   "PlumeLithosphereInteraction_passive_tracers.pvtu";
end

print("Postprocessing passive tracer files $FileName \n")

# Go over all Timestep subdirectories in the current directory
Directories         =   searchdir(pwd(),"Timestep_")

nMax                =   length(Directories)
#nMax                =   20;
minZ                =   [];
for iStep=1:nMax
    local DirName, data, points_coord, ID, Active, P,T,zCoord, New, id, Time
    local OutFileName;
    global minZ, OutNames, Time_vec;

    DirName     =   Directories[iStep];
    print("Processing directory $DirName \n")

    # Extract the timestep info from the directory directory name 
    id      =   findlast("_",DirName);
    Time    =   parse(Float64,DirName[id[1]+1:length(DirName)]);

    # ------------------------------------------------------------------------------------- 
    # Read data from timestep
    data        =   Read_VTU_File(DirName, FileName);
    
    points_coord =  ReadField_VTU(data,"coords");                   # coordinate array
    ID          =   ReadField_VTU(data,"ID");                       # ID of all points
    Active      =   ReadField_VTU(data,"Active");                   # Active or not?
    P           =   ReadField_VTU(data,"Pressure [MPa]");           # Pressure
    T           =   ReadField_VTU(data,"Temperature [C]");          # Temperature
    zCoord      =   points_coord[:,3];   

    if iStep==1
        minZ        =   zCoord;
        OutNames    =   Array{String}(undef,    nMax);
        Time_vec    =   Array{Float64}(undef,   nMax);
    end 

    # filter out data with -Inf (that was kind of a LaMEM hack to mark of )
    id          =   findall(x->x<-1e4, zCoord);
    if sizeof(id)>0
        Active[id]  .=   0;
    end

    # Determine minimum coordinate of active particles
    id          =   findall(x->x==1, Active);
    if sizeof(id)>0
        minZ[id]   =   min.(minZ[id],zCoord[id]);                   # obtain minimum of evert point in array
    end

    minZ .= minZ .+ zCoord

    #minZ_coord = minimum(minZ);
    #print("minimum Z coordinate = $minZ_coord \n")



    # Save data to new files --------------------------------------------------------------
   
    # Add data to object
    New = [];
    New = AddNewField_VTU(New, data, zCoord, "zCoord");        
    New = AddNewField_VTU(New, data, minZ,   "minZ");
    New = AddNewField_VTU(New, data, Active, "Active");             # will overwrite the previous "Active" field in file
   
    # write file back to disk
    OutFileName     =   "PassiveTracers_modified.vtu"
    WriteNewPoints_VTU(DirName, OutFileName, data, New);
    
    # Store filename and timestep info (for PVD files) ------------------------------------
    Time_vec[iStep]     =   Time;
    OutNames[iStep]     =   "$DirName/$OutFileName";

end

# Generate PVD file from the directories & filenames ----------------------------------
WritePVD_File("PassiveTracers_modified.pvd", OutNames, Time_vec);



