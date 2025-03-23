from django.db import models
from django.utils.translation import gettext_lazy as _


class Node(models.Model):
    class NodeType(models.TextChoices):
        HUB = 'hub', _('Hub principal')
        RELAY = 'relay', _('Relais intermédiaire')
        CLIENT = 'client', _('Client final')

    name = models.CharField(max_length=100)
    node_type = models.CharField(max_length=10, choices=NodeType.choices)
    latitude = models.FloatField()
    longitude = models.FloatField()
    capacity = models.IntegerField()

    def __str__(self):
        return f"{self.name} ({self.node_type})"


class Edge(models.Model):
    class RoadType(models.IntegerChoices):
        ASPHALTE = 0, _('Asphalte')
        LATERITE = 1, _('Latérite')
        PISTE = 2, _('Piste')

    source = models.ForeignKey(Node, related_name='edges_from', on_delete=models.CASCADE)
    destination = models.ForeignKey(Node, related_name='edges_to', on_delete=models.CASCADE)
    distance = models.FloatField(help_text="Distance en kilomètres")
    base_time = models.FloatField(help_text="Temps de parcours nominal en minutes")
    cost = models.FloatField(help_text="Coût monétaire de la traversée")
    road_type = models.IntegerField(choices=RoadType.choices)
    reliability = models.FloatField(help_text="Fiabilité [0-1]")
    restrictions = models.IntegerField(help_text="Ex: 1 = fragile, 2 = express, 4 = poids_max")
    time_variation_morning = models.FloatField(default=1.0)
    time_variation_afternoon = models.FloatField(default=1.0)
    time_variation_night = models.FloatField(default=1.0)

    def __str__(self):
        return f"{self.source.name} → {self.destination.name}"


class Vehicle(models.Model):
    class VehicleType(models.TextChoices):
        CAMION = 'camion', _('Camion')
        UTILITAIRE = 'utilitaire', _('Utilitaire')
        REFRIGERE = 'refrigere', _('Réfrigéré')

    type = models.CharField(max_length=20, choices=VehicleType.choices)
    capacity = models.IntegerField()
    availability_start = models.TimeField()
    availability_end = models.TimeField()
    cost_per_km = models.FloatField(help_text="Coût en FCFA par km")

    def __str__(self):
        return f"{self.get_type_display()} ({self.capacity} kg)"


class DeliveryRequest(models.Model):
    class Priority(models.TextChoices):
        STANDARD = 'standard', _('Standard')
        EXPRESS = 'express', _('Express')
        FRAGILE = 'fragile', _('Fragile')

    origin = models.ForeignKey(Node, related_name='delivery_origins', on_delete=models.CASCADE)
    destination = models.ForeignKey(Node, related_name='delivery_destinations', on_delete=models.CASCADE)
    volume = models.IntegerField()
    priority = models.CharField(max_length=10, choices=Priority.choices)
    deadline = models.TimeField()

    def __str__(self):
        return f"{self.priority.capitalize()} {self.origin.name} → {self.destination.name}"
