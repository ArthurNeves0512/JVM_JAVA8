class Animal {
    public void respirar() {
        System.out.println("Respirando");
    }
}

class Cachorro extends Animal {
}

public class Heranca {
    public static void main(String[] args) {
        Cachorro c = new Cachorro();
        c.respirar();
    }
}