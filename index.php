<?php
// Active les rapports d'erreurs pour un meilleur débogage (à désactiver en production)
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

// Paramètres de connexion à la base de données
$host = 'localhost';
$db   = 'bdd_rfid';
$user = 'user'; // Utilisez un utilisateur avec des privilèges limités pour la production
$pass = 'root';
$charset = 'utf8mb4';

// Options de la connexion PDO pour une meilleure sécurité
$options = [
    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    PDO::ATTR_EMULATE_PREPARES   => false,
];

// Création de la connexion PDO
$dsn = "mysql:host=$host;dbname=$db;charset=$charset";
try {
    $pdo = new PDO($dsn, $user, $pass, $options);
} catch (\PDOException $e) {
    throw new \PDOException($e->getMessage(), (int)$e->getCode());
}

// Vérification et nettoyage de l'entrée
$mode = $_REQUEST['mode'] ?? '';
$uid = $_REQUEST['uid'] ?? '';

if (!$uid) {
    echo "Aucune donnée fournie";
    exit;
}

if ($mode == 'write') {
    // Insertion de l'UID dans la base de données
    $stmt = $pdo->prepare("INSERT INTO cartes_rfid (identifiant_rfid) VALUES (:uid)");
    $stmt->execute(['uid' => $uid]);
    echo "UID enregistré avec succès";
} elseif ($mode == 'read') {
    // Vérification de l'existence de l'UID dans la base de données
    $stmt = $pdo->prepare("SELECT * FROM cartes_rfid WHERE identifiant_rfid = :uid");
    $stmt->execute(['uid' => $uid]);

    if ($stmt->rowCount() > 0) {
        echo "True";
    } else {
        echo "False";
    }
} else {
    echo "Mode non valide";
}
?>
