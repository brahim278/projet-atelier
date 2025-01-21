#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

#define MAX_NODES 10
#define MAX_MSG_SIZE 256
#define TOKEN "TOKEN"

// Structure pour les pipes entre les nœuds
struct NodePipes {
    int read_fd;
    int write_fd;
};

// Fonction pour colorer la sortie
void print_colored(int node_id, const char *message) {
    printf("\033[%dm[Node %d] %s\033[0m\n", 
           31 + (node_id % 6), node_id, message);
}

// Simulation d'un nœud
void node_process(int node_id, int total_nodes, struct NodePipes pipe) {
    char buffer[MAX_MSG_SIZE];
    char msg[MAX_MSG_SIZE];
    srand(time(NULL) + node_id);

    while (1) {
        // Attendre le jeton
        ssize_t bytes_read = read(pipe.read_fd, buffer, MAX_MSG_SIZE);
        if (bytes_read <= 0) break;

        sprintf(msg, "A reçu le jeton");
        print_colored(node_id, msg);

        // 30% de chance d'avoir des données à transmettre
        if (rand() % 100 < 30) {
            sprintf(msg, "Transmission de données en cours...");
            print_colored(node_id, msg);
            usleep(500000);
        }

        // Passer le jeton
        sprintf(msg, "Passe le jeton au nœud %d", 
                (node_id + 1) % total_nodes);
        print_colored(node_id, msg);
        write(pipe.write_fd, TOKEN, strlen(TOKEN) + 1);
        usleep(200000);
    }

    close(pipe.read_fd);
    close(pipe.write_fd);
    exit(0);
}

int main() {
    int total_nodes;
    
    printf("\033[1m\nSimulateur de Token Ring\033[0m\n");
    printf("Entrez le nombre de nœuds (max %d): ", MAX_NODES);
    scanf("%d", &total_nodes);

    if (total_nodes <= 0 || total_nodes > MAX_NODES) {
        printf("Erreur: nombre de nœuds invalide\n");
        return 1;
    }

    int pipes[MAX_NODES][2];
    for (int i = 0; i < total_nodes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Erreur création pipe");
            return 1;
        }
    }

    printf("\n\033[1mDémarrage du réseau Token Ring avec %d nœuds\033[0m\n\n", 
           total_nodes);

    for (int i = 0; i < total_nodes; i++) {
        pid_t pid = fork();
        
        if (pid == -1) {
            perror("Erreur fork");
            return 1;
        }
        
        if (pid == 0) {
            struct NodePipes node_pipe;
            node_pipe.read_fd = pipes[i][0];
            node_pipe.write_fd = pipes[(i + 1) % total_nodes][1];

            for (int j = 0; j < total_nodes; j++) {
                if (j != i) close(pipes[j][0]);
                if (j != (i + 1) % total_nodes) close(pipes[j][1]);
            }

            node_process(i, total_nodes, node_pipe);
            return 0;
        }
    }

    for (int i = 1; i < total_nodes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    sleep(1);
    printf("\033[1mInjection du jeton initial dans le réseau\033[0m\n\n");
    write(pipes[0][1], TOKEN, strlen(TOKEN) + 1);

    printf("\nAppuyez sur Enter pour terminer la simulation...\n");
    getchar();
    getchar();

    printf("\n\033[1mArrêt du réseau Token Ring\033[0m\n");
    for (int i = 0; i < total_nodes; i++) {
        kill(0, SIGTERM);
    }

    while (wait(NULL) > 0);
    return 0;
}
