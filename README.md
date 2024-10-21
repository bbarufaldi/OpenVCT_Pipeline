# **OpenVCT** 

![OpenVCT Logo](figs/logo.png)  
*Virtual Clinical Trials Platform for Medical Imaging*

---

## **Project Overview**  
The **OpenVCT** project provides a unified platform for conducting **Virtual Clinical Trials (VCTs)** in medical imaging. It includes tools for simulating patient accrual, reader studies, and generating cohorts of patients and medical images. To ensure cross-system compatibility, OpenVCT leverages standard data formats like **DICOM**.

This project is an updated, containerized implementation of the original version hosted on SourceForge ([link here](https://sourceforge.net/projects/openvct/)). We use **Docker** as the development platform for our applications.

For more details on the methodology behind OpenVCT, please refer to the following publications:

- Barufaldi, B., Bakic, P. R., Higginbotham, D., Maidment, A. D. A. (2018). OpenVCT: a GPU-accelerated virtual clinical trial pipeline for mammography and digital breast tomosynthesis. SPIE Medical Imaging 2018, 1057358(March), 10573588. [https://doi.org/10.1117/12.2294935](https://doi.org/10.1117/12.2294935)

- Barufaldi, B., Maidment, A. D. A., Dustler, M., Axelsson, R., Tomic, H., Zackrisson, S., Tingberg, A.,Bakic, P. R. (2021). Virtual Clinical Trials in Medical Imaging System Evaluation and Optimisation. Radiation Protection Dosimetry, 195 (3–4), 363–371. [https://doi.org/10.1093/rpd/ncab080](https://doi.org/10.1093/rpd/ncab080)

---

## **Installation and Execution**

OpenVCT is containerized for simplicity. Follow the steps below to get started:

```bash
# 1. Install CUDA Toolkit:
https://developer.nvidia.com/cuda-toolkit

# 2. Install WSL* (Linux distro for Windows) and Docker Desktop:
https://learn.microsoft.com/en-us/windows/wsl/install
https://www.docker.com/

## *Skip the WSL installation for Linux SO

# 3. Clone our repository:
git clone https://github.com/bbarufaldi/OpenVCT_Pipeline.git

# 4. Run the following command on your OPENVCT_PIPELINE directory: 
docker build -t openvct .

# 5. Compile the OpenVCT code:
docker run -it --ipc=host -v ${PWD}:/app/ --gpus all openvct python3 compile_pipeline.py

# 6. Create your 'own scripts' to run the steps of our pipeline (follow steps described below). Execute the pipeline using the command:
docker run -it --ipc=host -v ${PWD}:/app/ --gpus all openvct python3 example.py
```

We suggest you to build your own code following the steps described in **example.py** . 

---

## **System Requirements**

- **NVIDIA GPU** (VRAM > 4GB preferred)
- **CUDA Toolkit** version > 8

---

## **Pipeline Documentation**

### **Headers, Toolkit, and OpenVCT._helpers**

**Toolkit**: The toolkit is necessary for compiling the C++ version from the SourceForge implementation ([link here](https://sourceforge.net/projects/openvct/)), including ZipLib, which is used to compress phantoms with LZMA compression.

**Constants**: Defines various constants for phantom properties (e.g., dimension, voxel size, volume), deformation modes (CC or ML compression), and system configurations (acquisition, geometry, exposure settings, etc.).

**Readers, writers, and wrappers**: Auxiliary methods for reading and writing text files as well as methods convert to different file formats.

### **1. OpenVCT.anatomy**  
This **GPU-accelerated** step creates breast phantoms. You'll need to define the phantom configuration as follows:

```python
# Set up the XML file
xml = gen.XMLWriter(config=breast.BreastConfig.CUP_C, 
                    phantom_name="PhantomC",
                    xml_file = "./xml/PhantomC.xml")

# Generate phantom
subprocess.call(["./BreastPhantomGenerator_docker", "-xml_input", xml.xml_file])
```

<i>References for methodology</i>:

Pokrajac, D. D., Maidment, A. D. A., Bakic, P. R. (2012). Optimized generation of high resolution breast anthropomorphic software phantoms. Medical Physics, 39 (4), 2290–2302. [https://doi.org/10.1118/1.3697523](https://doi.org/10.1118/1.3697523)

### **2. OpenVCT.deform**

This **GPU-accelerated** step deforms (i.e., compresses) the breast phantoms. The deformation can be applied for standard mammographic compression views (CC or ML). Below is an example of how to configure the phantom deformation:

```python
# Set up the XML file for deformation
xml = defo.XMLWriter(config=deform.DeformerConfig.CUPC_CC, 
                     in_phantom = "../anatomy/vctx/PhantomC.vctx",
                     out_phantom = "./vctx/PhantomC.vctx",
                     xml_file = "./xml/PhantomC.xml")

# Run the deformation process
subprocess.call(["xvfb-run", "-s", "-screen 0 800x600x24", "python3", "VolumeDeformer.py", xml.xml_file])
```

<i>References for methodology</i>: 

Lago, M. A., Maidment, Andrew. D. A., Bakic, P. R. (2013). Modelling of mammographic compression of anthropomorphic software breast phantom using FEBio. Intl Symposium on Computer Methods in Biomechanics and Biomedical Engineering.

Barufaldi, B., Bakic, P. R., Pokrajac, D. D., Lago, M. A., Maidment, A. D. A. (2018). Developing populations of software breast phantoms for virtual clinical trials. 14th International Workshop on Breast Imaging (IWBI 2018), July, 73. [https://doi.org/10.1117/12.2318473](https://doi.org/10.1117/12.2318473)

### **3. OpenVCT.insert**

This method allows insertion of **lesions** (e.g., calcifications, masses) into breast phantoms using voxel addition (composite materials) or replacement.

```python
# Example lesion insertion
xml = ins.Inserter(in_phantom = "../deform/vctx/PhantomC.vctx",
                   out_phantom = "./vctx/PhantomC.vctx",
                   xml_file = "./xml/PhantomC.xml",
                   num_lesions = 2,
                   size_mm = [(13, 13, 7)],
                   db_dir='db/mass')

# Optional weight parameter
subprocess.call(["python3", "./LesionInserter.py", xml.xml_file, '0.3'])
```

<i>References for methodology</i>: 

Barufaldi, B., Vent, T. L., Bakic, P. R., Maidment, A. D. A. (2022). Computer simulations of case difficulty in digital breast tomosynthesis using virtual clinical trials. Med. Phys, 49(4), 2220–2232. [https://doi.org/10.1002/mp.15553](https://doi.org/10.1002/mp.15553)

Barufaldi, B., Acciavatti, R. J., Conant, E. F., Maidment, A. D. A. (2023). Impact of super-resolution and image acquisition on the detection of calcifications in digital breast tomosynthesis. European Radiology, 1–11. [https://doi.org/10.1007/s00330-023-10103-6](https://doi.org/10.1007/s00330-023-10103-6)

### **4. OpenVCT.raytracing**

Simulate x-ray images using GPU-accelerated raytracing methods. A noise model simulates clinical noise (electronic and quantum) using methods developed by the **LAVI-USP** group (Brazil, https://github.com/LAVI-USP).

```python
# Define ray tracing settings
xml = proj.XMLWriter(config=system.SystemConfig.HOLOGIC, 
                     phantom_name="../inserter/vctx/PhantomC.vctx", 
                     folder_name="./proj/PhantomC-proj",
                     xml_file = "./xml/PhantomC.xml")

# Run ray tracing
subprocess.call(["./XPLProjectionSim_GPU_docker", "-xml_input", xml.xml_file])

# Add clinical noise
noise = noise.NoiseModel(config=system.SystemConfig.HOLOGIC, 
                         input_folder="./raytracing/proj/PhantomC-proj",
                         output_folder="./noise/proj/PhantomC-proj")
noise.add_noise()
```

<i>References for methodology</i>:

Borges, L. R., Barufaldi, B., Caron, R. F., Bakic, P. R., Foi, A., Maidment, A. D. A., Vieira, M. A. C. (2019). Technical Note: Noise models for virtual clinical trials of digital breast tomosynthesis. Medical Physics, 46(6), 2683–2689. [https://doi.org/10.1002/mp.13534](https://doi.org/10.1002/mp.13534)

---

## **Disclaimer**

OpenVCT is an ongoing project and there are features and modules under development:

1) Lack of finer structures simulated in the breast. A simplex-based method has been implemented and will be integrated. [GitHub](https://github.com/jpvt/NoiseVolumesDemo)

2) Compressions are limited to CC and ML views; the pectoralis muscle is not simulated.

3) Scatter is not implemented in the ray tracing approach.

## **Licenses**

## **Contact**

For any inquiries or issues with the project, please reach out to our team's GitHub webpage (issues section):

X-ray Physics Laboratory (XPL) | [GitHub](https://github.com/upenn/xpl/xpl-OPENVCT_PIPELINE)

For other specific questions, contact: 

Bruno Barufaldi – Bruno.Barufaldi@pennmedicine.upenn.edu | [GitHub](https://github.com/bbarufaldi)

Andrew Maidment – Andrew.mMidment@pennmedicine.upenn.edu | [GitHub](https://github.com/maidment)

## **Contributors**

We are very thankful for all contributors to the version of this project:

Predrag R. Bakic - predrag.bakic@med.lu.se

Marcelo Vieira and LAVI-USP - mvieira@sc.usp.br | [GitHub](https://github.com/LAVI-USP)

Miguel Lago and FDA (VICTRE Project) - miguel.lago@fda.hhs.gov | [GitHub](https://github.com/malago86)

## **Funding Sources**

Terri-Brodeur Breast Cancer Foundation with grant 2023 TBBCF  (PI: Barufaldi)

Susan G. Komen Foundation with grant 2022 CCR231010477 (PI: Barufaldi)