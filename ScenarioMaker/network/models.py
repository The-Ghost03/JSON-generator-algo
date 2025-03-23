from django.db import models
from django.utils.translation import gettext_lazy as _

class Noeud(models.Model):
    class TypeNoeud(models.TextChoices):
        HUB = 'hub', _('Hub principal')
        RELAIS = 'relais', _('Relais intermédiaire')
        CLIENT = 'client', _('Client final')

    nom = models.CharField(max_length=100)
    type_noeud = models.CharField(max_length=10, choices=TypeNoeud.choices)
    latitude = models.FloatField()
    longitude = models.FloatField()
    capacite = models.IntegerField()

    def __str__(self):
        return f"{self.nom} ({self.type_noeud})"


class Arete(models.Model):
    class TypeRoute(models.IntegerChoices):
        ASPHALTE = 0, _('Asphalte')
        LATERITE = 1, _('Latérite')
        PISTE = 2, _('Piste')

    source = models.ForeignKey(Noeud, related_name='aretes_depuis', on_delete=models.CASCADE)
    destination = models.ForeignKey(Noeud, related_name='aretes_vers', on_delete=models.CASCADE)
    distance = models.FloatField(help_text="Distance en kilomètres")
    temps_base = models.FloatField(help_text="Temps de parcours nominal en minutes")
    cout = models.FloatField(help_text="Coût monétaire de la traversée")
    type_route = models.IntegerField(choices=TypeRoute.choices)
    fiabilite = models.FloatField(help_text="Fiabilité [0-1]")
    restrictions = models.IntegerField(help_text="Ex : 1 = fragile, 2 = express, 4 = poids maximum")
    variation_temps_matin = models.FloatField(default=1.0)
    variation_temps_apresmidi = models.FloatField(default=1.0)
    variation_temps_nuit = models.FloatField(default=1.0)

    def __str__(self):
        return f"{self.source.nom} → {self.destination.nom}"


class Vehicule(models.Model):
    class TypeVehicule(models.TextChoices):
        CAMION = 'camion', _('Camion')
        UTILITAIRE = 'utilitaire', _('Utilitaire')
        REFRIGERE = 'refrigere', _('Réfrigéré')

    type_vehicule = models.CharField(max_length=20, choices=TypeVehicule.choices)
    capacite = models.IntegerField()
    disponibilite_debut = models.TimeField()
    disponibilite_fin = models.TimeField()
    cout_par_km = models.FloatField(help_text="Coût en FCFA par km")

    def __str__(self):
        return f"{self.get_type_vehicule_display()} ({self.capacite} kg)"


class DemandeLivraison(models.Model):
    class Priorite(models.TextChoices):
        STANDARD = 'standard', _('Standard')
        EXPRESS = 'express', _('Express')
        FRAGILE = 'fragile', _('Fragile')

    origine = models.ForeignKey(Noeud, related_name='livraisons_origine', on_delete=models.CASCADE)
    destination = models.ForeignKey(Noeud, related_name='livraisons_destination', on_delete=models.CASCADE)
    volume = models.IntegerField()
    priorite = models.CharField(max_length=10, choices=Priorite.choices)
    echeance = models.TimeField()

    def __str__(self):
        return f"{self.priorite.capitalize()} {self.origine.nom} → {self.destination.nom}"
