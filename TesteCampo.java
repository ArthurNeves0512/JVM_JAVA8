class Animal {
    int idade;

    void setIdade(int i) {
        idade = i;
    }

    void imprime() {
        System.out.println(idade);
    }
}

public class TesteCampo {
    public static void main(String[] args) {
        Animal a = new Animal();

        a.setIdade(10);
        a.imprime();
    }
}