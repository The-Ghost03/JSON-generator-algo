# Generated by Django 5.1.7 on 2025-03-23 19:19

import django.db.models.deletion
from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('network', '0001_initial'),
    ]

    operations = [
        migrations.CreateModel(
            name='Arete',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('distance', models.FloatField(help_text='Distance en kilomètres')),
                ('temps_base', models.FloatField(help_text='Temps de parcours nominal en minutes')),
                ('cout', models.FloatField(help_text='Coût monétaire de la traversée')),
                ('type_route', models.IntegerField(choices=[(0, 'Asphalte'), (1, 'Latérite'), (2, 'Piste')])),
                ('fiabilite', models.FloatField(help_text='Fiabilité [0-1]')),
                ('restrictions', models.IntegerField(help_text='Ex : 1 = fragile, 2 = express, 4 = poids maximum')),
                ('variation_temps_matin', models.FloatField(default=1.0)),
                ('variation_temps_apresmidi', models.FloatField(default=1.0)),
                ('variation_temps_nuit', models.FloatField(default=1.0)),
            ],
        ),
        migrations.CreateModel(
            name='DemandeLivraison',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('volume', models.IntegerField()),
                ('priorite', models.CharField(choices=[('standard', 'Standard'), ('express', 'Express'), ('fragile', 'Fragile')], max_length=10)),
                ('echeance', models.TimeField()),
            ],
        ),
        migrations.CreateModel(
            name='Noeud',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('nom', models.CharField(max_length=100)),
                ('type_noeud', models.CharField(choices=[('hub', 'Hub principal'), ('relais', 'Relais intermédiaire'), ('client', 'Client final')], max_length=10)),
                ('latitude', models.FloatField()),
                ('longitude', models.FloatField()),
                ('capacite', models.IntegerField()),
            ],
        ),
        migrations.CreateModel(
            name='Vehicule',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('type_vehicule', models.CharField(choices=[('camion', 'Camion'), ('utilitaire', 'Utilitaire'), ('refrigere', 'Réfrigéré')], max_length=20)),
                ('capacite', models.IntegerField()),
                ('disponibilite_debut', models.TimeField()),
                ('disponibilite_fin', models.TimeField()),
                ('cout_par_km', models.FloatField(help_text='Coût en FCFA par km')),
            ],
        ),
        migrations.RemoveField(
            model_name='deliveryrequest',
            name='destination',
        ),
        migrations.RemoveField(
            model_name='deliveryrequest',
            name='origin',
        ),
        migrations.RemoveField(
            model_name='edge',
            name='destination',
        ),
        migrations.RemoveField(
            model_name='edge',
            name='source',
        ),
        migrations.DeleteModel(
            name='Vehicle',
        ),
        migrations.AddField(
            model_name='demandelivraison',
            name='destination',
            field=models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='livraisons_destination', to='network.noeud'),
        ),
        migrations.AddField(
            model_name='demandelivraison',
            name='origine',
            field=models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='livraisons_origine', to='network.noeud'),
        ),
        migrations.AddField(
            model_name='arete',
            name='destination',
            field=models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='aretes_vers', to='network.noeud'),
        ),
        migrations.AddField(
            model_name='arete',
            name='source',
            field=models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='aretes_depuis', to='network.noeud'),
        ),
        migrations.DeleteModel(
            name='DeliveryRequest',
        ),
        migrations.DeleteModel(
            name='Edge',
        ),
        migrations.DeleteModel(
            name='Node',
        ),
    ]
