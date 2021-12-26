# Guía de uso e instalación

```console
# CONFIGURAR LA TOPOLOGIA DEl CLUSTER, SIGUIENDO EL ARCHIVO MAQUETA DE DVS
# hacer lo siguiente para cada nodo N de la topologia: 
# cambiar hostname
echo 'nodeN' > /etc/hostname
# modifiar interfaz de red en /etc/network/interfaces
# el archivo deberia contener (N es el numero de nodo):
auto lo
iface lo inet loopback
auto eth0
iface eth0 inet static
address 192.168.0.10N
netmask 255.255.255.0
network 192.168.0.0
broadcast 192.168.0.255
# checkear que las direcciones anteriores coinciden con la red en la que se encuentra  la pc host,
# por ejemplo, dependiendo del proveedor, puede que la red sea del estilo 192.168.100.0, 192.168.1.0, etc
# en caso de ser necesario, modificar el archivo /etc/hosts
# reiniciar la vm
# probar el ping entre los nodos
# probar ping a www.google.com desde los nodos
# probar ping y ssh desde la pc host


# CONFIGURAR PASSWORDLESS SSH PARA NODE0 CON TODOS LOS NODOS
# es probable que haya que borrar claves  ssh que ya existen en los nodos
# seguir los pasos en 
# https://linuxize.com/post/how-to-setup-passwordless-ssh-login/
# probar ssh desde node0 a los demas nodos. La idea es que no pida contrasenia


# CONFIGURAR CARPETA NFS (MANAGER) EN NODE0
# https://mpitutorial.com/tutorials/running-an-mpi-cluster-within-a-lan/
# Instalar nfs
sudo apt-get install nfs-kernel-server
# Crear el directorio compartido
mkdir /root/cloud
# Agregar una linea al archivo /etc/exports:
echo '/root/cloud *(rw,sync,no_root_squash,no_subtree_check)' >> /etc/exports
# Ver que todo este en orden
cat /etc/exports
# Iniciar nfs
exportfs -a
sudo service nfs-kernel-server restart


# CONFIGURAR CARPETA NFS (WORKER) EN NODE1, NODE2, NODE3...
# instalar nfs
sudo apt-get install nfs-common
# crear el directorio
mkdir /root/cloud
# montar el directorio por nfs
sudo mount -t nfs node0:/root/cloud /root/cloud
# ver que el directorio se haya montado correctamente
df -h
# agregar una linea a fstab para que montar automaticamente
echo 'node0:/root/cloud /root/cloud nfs' >> /etc/fstab
# nota: siempre arrancar node0 primero para que los demas nodos 
# puedan montar por NFS


# INSTALACION DE TIPC
# deberia venir instalado con Linux
# agregar las siguientes lineas al ~/.bashrc de cada nodo
# nota: M = N+1. Ejemplo: node0 -> M=1
tipc node set address 1.1.M
tipc bearer enable media eth device eth0
# verificar el estado de la topologia TIPC
tipc link list
tipc nametable show


# INSTALACION DE OMPI CON COMPONENTE TIPC 
# en node0, clonar este repositorio
cd /root/cloud
git clone --recursive https://github.com/fideldalmasso/ompi.git
cd ompi
# configurar y compilar ompi
./autogen.pl &> salida_autogen.txt
# revisar salida_autogen.txt, no deberia finalizar con errores
./configure --prefix=/root/cloud/openmpi --enable-debug --enable-static=yes --with-devel-headers=1 &> salida_configure.txt
# revisar salida_configure.txt, no deberia finalizar con errores
make -j6 all install --&> salida_make.txt
# revisar salida_make.txt, no deberia finalizar con errores
# los binarios se encontraran en /root /cloud/openmpi, 
# que no es lo mismo que /root/cloud/ompi
# verificar que el modulo de TIPC aparece en la lista
ompi_info | grep 'MCA [mb]tl'
# agregar las siguientes lineas al ~/.bashrc de cada nodo
PATH="/root/cloud/openmpi:$PATH"
export OPAL_PREFIX=/root/cloud/openmpi
export OMPI_ALLOW_RUN_AS_ROOT=1
export OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1


# FINALMENTE, REINICIAR LAS SESIONES DE TERMINAL
source ~/.bashrc


# PROBAR OPENMPI CON TIPC EN UN PROYECTO DE PRUEBAS
# clonar el repositorio rtiow
cd /root/cloud
git clone https://github.com/fideldalmasso/rtiow
cd rtiow
# compilar y ejecutar el proyecto
make
mpirun -v -np 2 --hostfile host_file ./rtiow_distribuido.bin --mca btl tipc,self --mca btl_base_verbose 100


# PROBAR OPENMPI CON TCP EN UN PROYECTO DE PRUEBAS
# modificar el archivo /root/cloud/ompi/opal/mca/btl/tipc/btl_tipc.h
# comentar/quitar la linea 19:
    #define FIDEL 1
# recompilar ompi 
cd /root/cloud/ompi
make -j6 all install &> salida_make.txt
# correr rtiow de la misma forma que antes
cd /root/cloud/rtiow
make
mpirun -v -np 4 --hostfile host_file ./rtiow_distribuido.bin --mca btl tipc,self --mca btl_base_verbose 100


# DEBUGGEAR PROYECTO DE PRUEBAS CON GDB
# METODO CON LOGS
# posicionarse en la rama debug del proyecto de pruebas
cd /root/cloud/rtiow
git checkout debug
mkdir log
# correr el programa normalmente 
mpirun -v -np 2 --hostfile host_file ./rtiow_distribuido.bin --mca btl tipc,self --mca btl_base_verbose 100
# el programa quedará en un bucle infinito y mostrará una serie de PIDs
# abrir las terminales que sean necesarias y correr gdb en cada una de ellas
gdb --command=comando1.txt -p 8639
# aqui 8639 corresponde a unos de los PIDs mostrados en pantalla
# correr gdb con estos parametros hara que se generen archivos de log dentro del directorio log/
# cada linea representa una llamada a una funcion dentro del componente TIPC

# METODO CON GUI
# una alternativa es usar gdbgui, que ofrece una gui del debugger a traves del navegador
gdbgui -r --gdb-cmd "gdb --command=comando2.txt -p 8639"



```
# Notas extra
```console
# COMPILAR UN PROGRAMA C++ CON OPENMPI
# https://curc.readthedocs.io/en/latest/programming/MPI-C.html
mpic++ hello_world_mpi.cpp -o hello

# COMANDOS UTILES PARA OMPI
ompi_info | grep 'MCA [mb]tl'
ompi_info --param btl tcp


# ALGUNAS VARIABLES DE DEBUGGING PARA AGREGAR EN ~/.bashrc 
export OMPI_MCA_mpi_param_check=1
export OMPI_MCA_mpi_show_handle_leaks=1
export OMPI_MCA_mpi_show_mca_params=1
export OMPI_MCA_mpi_show_mca_params_file=1
export OMPI_MCA_mpi_keep_peer_hostnames=1
export OMPI_MCA_mpi_abort_delay=1
export OMPI_MCA_mpi_abort_print_stack=1


```