// Universidad de Carabobo - Arquitectura del Computador
// Proyecto Final 14: Simulador de Caché con Menú Interactivo y Comparativa
// Daniel Gutierrez CI: 32047059    Gabriel Diaz CI: 31161550

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <cstdint>

using namespace std;

struct LineaCache {
    bool valida = false;
    uint64_t tag = 0;
    uint64_t contador_lru = 0; 
};

struct ConjuntoCache {
    vector<LineaCache> lineas;
};

// Estructura para almacenar los resultados y armar la comparativa
struct ResultadosSimulacion {
    uint32_t tamano_linea;
    double hit_rate;
    double miss_rate;
    double amat;
};

class SimuladorCache {
private:
    vector<ConjuntoCache> conjuntos;
    uint32_t bits_offset;
    uint32_t bits_index;
    uint64_t mascara_index;
    
    uint64_t reloj_global = 0;
    uint64_t total_accesos = 0;
    uint64_t hits = 0;
    uint64_t misses = 0;

public:
    SimuladorCache(uint64_t tamano_total, uint32_t vias, uint32_t tamano_linea) {
        uint64_t total_lineas = tamano_total / tamano_linea;
        uint64_t num_conjuntos = total_lineas / vias;

        bits_offset = log2(tamano_linea);
        bits_index = log2(num_conjuntos);
        mascara_index = num_conjuntos - 1;

        conjuntos.resize(num_conjuntos);
        for (auto& conjunto : conjuntos) {
            conjunto.lineas.resize(vias);
        }
    }

    void procesar_acceso(uint64_t direccion) {
        reloj_global++;
        total_accesos++;

        uint64_t index = (direccion >> bits_offset) & mascara_index;
        uint64_t tag = direccion >> (bits_offset + bits_index);

        ConjuntoCache& conjunto = conjuntos[index];
        bool hit = false;

        for (auto& linea : conjunto.lineas) {
            if (linea.valida && linea.tag == tag) {
                hits++;
                linea.contador_lru = reloj_global;
                hit = true;
                break;
            }
        }

        if (!hit) {
            misses++;
            int indice_reemplazo = -1;

            for (size_t i = 0; i < conjunto.lineas.size(); ++i) {
                if (!conjunto.lineas[i].valida) {
                    indice_reemplazo = i;
                    break;
                }
            }

            if (indice_reemplazo == -1) {
                uint64_t lru_mas_viejo = conjunto.lineas[0].contador_lru;
                indice_reemplazo = 0;
                for (size_t i = 1; i < conjunto.lineas.size(); ++i) {
                    if (conjunto.lineas[i].contador_lru < lru_mas_viejo) {
                        lru_mas_viejo = conjunto.lineas[i].contador_lru;
                        indice_reemplazo = i;
                    }
                }
            }

            conjunto.lineas[indice_reemplazo].valida = true;
            conjunto.lineas[indice_reemplazo].tag = tag;
            conjunto.lineas[indice_reemplazo].contador_lru = reloj_global;
        }
    }

    ResultadosSimulacion generar_resultados(uint32_t tamano_linea) {
        ResultadosSimulacion res;
        res.tamano_linea = tamano_linea;
        res.hit_rate = (total_accesos > 0) ? ((double)hits / total_accesos * 100.0) : 0;
        res.miss_rate = 100.0 - res.hit_rate;
        res.amat = 1.0 + ((res.miss_rate / 100.0) * 100.0); 
        return res;
    }

    void reporte_individual(uint32_t tamano_linea) {
        ResultadosSimulacion res = generar_resultados(tamano_linea);
        cout << "\n---------------------------------------\n";
        cout << "Tamano de Linea Configurado: " << tamano_linea << " bytes\n";
        cout << "Total Accesos              : " << total_accesos << "\n";
        cout << "Hits                       : " << hits << "\n";
        cout << "Misses                     : " << misses << "\n";
        cout << fixed << setprecision(2);
        cout << "Hit Rate                   : " << res.hit_rate << " %\n";
        cout << "Miss Rate                  : " << res.miss_rate << " %\n";
        cout << "AMAT Estimado              : " << res.amat << " ciclos\n";
        cout << "---------------------------------------\n";
    }
};

// Función auxiliar para correr una simulación y devolver los resultados
ResultadosSimulacion ejecutar_simulacion(uint32_t tamano_linea, const string& nombre_archivo, bool mostrar_detalle) {
    uint64_t tamano_cache = 32768; // 32 KB
    uint32_t vias = 4;             

    SimuladorCache cache(tamano_cache, vias, tamano_linea);
    ifstream archivo(nombre_archivo);
    
    if (!archivo.is_open()) {
        cerr << "Error al abrir la traza: " << nombre_archivo << "\n";
        return cache.generar_resultados(tamano_linea);
    }

    string linea_hex;
    while (archivo >> linea_hex) {
        try {
            uint64_t direccion = stoull(linea_hex, nullptr, 16);
            cache.procesar_acceso(direccion);
        } catch (...) {}
    }
    archivo.close();

    if (mostrar_detalle) {
        cache.reporte_individual(tamano_linea);
    }

    return cache.generar_resultados(tamano_linea);
}

int main() {
    string nombre_archivo;
    cout << "=======================================\n";
    cout << "  SIMULADOR DE CACHE - PROYECTO 14\n";
    cout << "=======================================\n";
    cout << "Ingrese el nombre del archivo de traza (ej. trace.txt): ";
    cin >> nombre_archivo;

    // Verificar que el archivo exista antes de mostrar el menú
    ifstream test_archivo(nombre_archivo);
    if (!test_archivo.is_open()) {
        cerr << "Error: No se encontro el archivo '" << nombre_archivo << "'. Ejecute de nuevo.\n";
        return 1;
    }
    test_archivo.close();

    int opcion = 0;
    while (opcion != 5) {
        cout << "\n=== MENU DE SIMULACION ===\n";
        cout << "1. Simular con linea de 16 bytes\n";
        cout << "2. Simular con linea de 64 bytes\n";
        cout << "3. Simular con linea de 128 bytes\n";
        cout << "4. Comparar todos los tamanos (16 vs 64 vs 128)\n";
        cout << "5. Salir\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        if (opcion == 1) {
            ejecutar_simulacion(16, nombre_archivo, true);
        } 
        else if (opcion == 2) {
            ejecutar_simulacion(64, nombre_archivo, true);
        } 
        else if (opcion == 3) {
            ejecutar_simulacion(128, nombre_archivo, true);
        } 
        else if (opcion == 4) {
            cout << "\nEjecutando simulaciones en lote...\n";
            ResultadosSimulacion r16 = ejecutar_simulacion(16, nombre_archivo, false);
            ResultadosSimulacion r64 = ejecutar_simulacion(64, nombre_archivo, false);
            ResultadosSimulacion r128 = ejecutar_simulacion(128, nombre_archivo, false);

            cout << "\n=======================================================\n";
            cout << "               TABLA COMPARATIVA DE RENDIMIENTO          \n";
            cout << "=======================================================\n";
            cout << left << setw(15) << "Tamano Linea" 
                 << setw(15) << "Hit Rate (%)" 
                 << setw(15) << "Miss Rate (%)" 
                 << setw(15) << "AMAT (ciclos)" << "\n";
            cout << "-------------------------------------------------------\n";
            cout << left << setw(15) << "16 bytes" 
                 << setw(15) << fixed << setprecision(2) << r16.hit_rate 
                 << setw(15) << r16.miss_rate 
                 << setw(15) << r16.amat << "\n";
            cout << left << setw(15) << "64 bytes" 
                 << setw(15) << r64.hit_rate 
                 << setw(15) << r64.miss_rate 
                 << setw(15) << r64.amat << "\n";
            cout << left << setw(15) << "128 bytes" 
                 << setw(15) << r128.hit_rate 
                 << setw(15) << r128.miss_rate 
                 << setw(15) << r128.amat << "\n";
            cout << "=======================================================\n";
        }
        else if (opcion != 5) {
            cout << "Opcion no valida. Intente de nuevo.\n";
        }
    }

    cout << "Saliendo del simulador...\n";
    return 0;
}