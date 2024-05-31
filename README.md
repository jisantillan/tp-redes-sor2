# Sistemas Operativos y Redes II

## <u>Trabajo Práctico Redes</u>

El objetivo del siguiente proyecto es realizar un análisis de redes, a partir de un escenario con una topología <b>Dumbbell</b> y conexiones cableadas.

Para ello tenemos 3 emisores on/off application (dos TCP y uno UDP), 3 receptores (misma distribución que los emisores) y dos routers intermedios.



![Dumbbell Img](img/dumbbell_img.png)

Click [aquí](informe/Informe.pdf) para ver el informe completo.


#### Instrucciones de ejecución
Los archivos con extensión .cc, deben copiarse en el directorio de trabajo de NS-3:
<i> /USER/ns3/ns3-allinone-3.31/ns3-3.31/scratch</i>

- Abrir una terminal y navegar a la carpeta que contiene el directorio scratch:
```
/USER/ns3/ns3-allinone-3.31/ns3-3.31
``` 
-  Ejecutar el siguiente comando para compilar y correr los archivos C++: 
```sh
    ./waf --run scratch/<name-of-file> 
```
- Reemplazar el <name-of-file> por el nombre del .cc. Por ejemplo:
```./waf --run scratch/dumbbell-topology-tcp-wireshark```

- Si la ejecución fué exitosa, veremos el mensaje al final de la consola.
```> build finished successfully```
