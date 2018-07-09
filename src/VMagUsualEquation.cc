#include "VMagUsualEquation.hh"

#include "aliases.hh"

#include <G4MagneticField.hh>
#include <globals.hh>
#include <G4PhysicalConstants.hh>

VMagUsualEquation::VMagUsualEquation(G4MagneticField* field):
    fCof(0),
    fBField(field)
{}

void VMagUsualEquation::SetChargeMomentumMass(G4ChargeState particleCharge,
                                              G4double momentumXc,
                                              G4double particleMass)
{
   G4double pcharge = particleCharge.GetCharge();
   fCof = pcharge * CLHEP::eplus * CLHEP::c_light;
}

Double_8v VMagUsualEquation::operator() (const Double_8v& y)
{
    G4double B[4];
    G4double point[4];
    point[0] = y[0];
    point[1] = y[1];
    point[2] = y[2];
    point[3] = y[7];
    fBField->GetFieldValue(point, B);

    Double_4v yPermutationA{y[3], y[4], y[5], 0};
    Double_4v yPermutationB{y[4], y[5], y[3], 0};
    Double_4v yPermutationC{y[5], y[3], y[4], 0};

    Double_4v BmagPermutationA{B[1], B[2], B[0], 0};
    Double_4v BmagPermutationB{B[2], B[0], B[1], 0};

    const G4double momentumMagSquare
            = vecCore::ReduceAdd(yPermutationA * yPermutationA);

    const G4double invMomentumMagnitude = 1.0 / std::sqrt(momentumMagSquare);

    const G4double cof = fCof * invMomentumMagnitude;

    Double_4v positionGradient = yPermutationA * invMomentumMagnitude;

    Double_4v momentumGradient = cof * (yPermutationB * BmagPermutationB
            - yPermutationC * BmagPermutationA);

    return {positionGradient[0], positionGradient[1], positionGradient[2], 
            momentumGradient[0], momentumGradient[1], momentumGradient[2],
            0, 0};
}
